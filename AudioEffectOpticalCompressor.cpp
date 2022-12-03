#include "AudioEffectOpticalCompressor.h"

void AudioEffectOpticalCompressor::init(float sampleRate) {
  this->sampleRate = sampleRate;

  // defaults
  setThresholdDb(-3.0);
  setBias(70.0);
  setMakeupGainDb(0.0);
  setBlownCapacitor(true);
  setTimeConstant(4);
  setRmsWindowMs(100);
}

void AudioEffectOpticalCompressor::update(void) {

  // work memory
  audio_block_t *inBlock;
  audio_block_t *outBlock;

  inBlock = receiveReadOnly();
  outBlock = allocate();

#ifdef _HAS_FPU
  float tmp;
#endif

  if (inBlock == NULL || outBlock == NULL) return;

  // do the compressing
  for (int i = 0; i != AUDIO_BLOCK_SAMPLES; ++i) {

    spl = inBlock->data[i];
    maxspl = spl * spl;

#ifndef _HAS_FPU
    runave = maxspl + rmscoef * (runave - maxspl);
#else
    runave = famf(rmscoef, runave - maxspl, maxspl);
#endif

    det = fastSqrt3(max(0, runave));

    overdb = capsc * fastLog(det * threshvRecip);
    overdb = max(0, overdb);

#ifndef _HAS_FPU
    if (overdb > rundb) {
      rundb = overdb + atcoef * (rundb - overdb);
    } else {
      rundb = overdb + relcoef * (rundb - overdb);
    }
#else
    if (overdb > rundb) {
      rundb = fmaf(atcoef , rundb - overdb, overdb);
    } else {
      rundb = fmaf(relcoef, rundb - overdb, overdb);
    }
#endif

    overdb = max(rundb, 0);

    if (bias == 0) {
      cratio = ratio;
    } else {

#ifndef _HAS_FPU
      cratio = 1 + (ratio - 1) * fastSqrt3((overdb + dcoffset) / (bias + dcoffset));
#else
      tmp = fastSqrt3((overdb + dcoffset) / (bias + dcoffset));
      cratio = fmaf(ratio - 1, tmp, 1);
#endif

    }

    gr = -overdb * (cratio - 1) / cratio;
    grv = fastExp(gr * DB_TO_LOG);

    spl *= grv * makeupv;
    outBlock->data[i] = spl;

  }

  // send the block and release the memory
  transmit(outBlock);

  // need to also release the input block because the library uses reference counting...
  release(inBlock);
  release(outBlock);

}

void AudioEffectOpticalCompressor::setThresholdDb(float thresh) {
  __disable_irq();
  this->thresh = thresh;
  threshv = exp(thresh * DB_TO_LOG);
  threshvRecip = 1.0 / threshv;
  setThresholdParams(thresh, bias);
  __enable_irq();
}

void AudioEffectOpticalCompressor::setBias(float bias) {
  __disable_irq();
  this->bias = 80 * bias * 0.01;
  setThresholdParams(thresh, bias);
  __enable_irq();
}

void AudioEffectOpticalCompressor::setThresholdParams(float thresh, float bias) {
  float cthresh = thresh - bias;
  cthreshv = exp(cthresh * DB_TO_LOG);
}

void AudioEffectOpticalCompressor::setMakeupGainDb(float gain) {
  __disable_irq();
  makeupv = exp(gain * DB_TO_LOG);
  __enable_irq();
}

void AudioEffectOpticalCompressor::setBlownCapacitor(bool blownCap) {
  __disable_irq();
  this->blownCap = blownCap;
  capsc = blownCap ? LOG_TO_DB : LOG_TO_DB * BLOWN_CAP_SCALAR;
  __enable_irq();
}

void AudioEffectOpticalCompressor::setTimeConstant(int tc) {
  __disable_irq();
  float attime, reltime;
  switch (tc) {
    default:
    case 1:
      attime = 0.0002;
      reltime = 0.300;
      break;

    case 2:
      attime = 0.0002;
      reltime = 0.800;
      break;

    case 3:
      attime = 0.0004;
      reltime = 2.000;
      break;

    case 4:
      attime = 0.0008;
      reltime = 5.000;
      break;

    case 5:
      attime = 0.0002;
      reltime = 10.000;
      break;

    case 6:
      attime = 0.0004;
      reltime = 25.000;
      break;
  }

  atcoef = exp(-1 / (attime * sampleRate));
  relcoef = exp(-1 / (reltime * sampleRate));

  __enable_irq();
}

void AudioEffectOpticalCompressor::setRmsWindowMs(int windowMs) {
  __disable_irq();
  float rmstime = (float)windowMs / 1000000;
  rmscoef = exp(-1 / (rmstime * sampleRate));
  __enable_irq();
}
