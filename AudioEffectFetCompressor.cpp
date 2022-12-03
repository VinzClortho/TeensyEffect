#include "AudioEffectFetCompressor.h"

void AudioEffectFetCompressor::init(float sampleRate) {
  this->sampleRate = sampleRate;
  ratioMode = BlownCap8;

  // set defaults
  setThresholdDb(-3.0);
  setRatioMode(BlownCap4);
  setGainDb(0.0);
  setAttackTime(100000);
  setReleaseTime(100);
  setMix(100.0);
  ratio = 0;
  cratio = 0;
  rundb = 0;
  overdb = 0;
  ratatcoef = fastExp(-1 / (0.00001 * sampleRate));
  ratrelcoef = fastExp(-1 / (0.5 * sampleRate));
}

void AudioEffectFetCompressor::update(void) {

  // work memory
  audio_block_t *inBlock;
  audio_block_t *outBlock;

  inBlock = receiveReadOnly();
  outBlock = allocate();

  if (inBlock == NULL || outBlock == NULL) return;

  // do the compressing
  for (int i = 0; i != AUDIO_BLOCK_SAMPLES; ++i) {

    spl = inBlock->data[i];
    ospl = spl;
    maxspl = spl * spl;
    runave = maxspl + rmscoef * (runave - maxspl);
    det = fastSqrt3(max(0, runave));
    overdb = max(0, capsc * fastLog(det * cthreshvRecip));

    if (overdb - rundb > 5) averatio = 4;

    if (overdb > rundb) {

#ifndef _HAS_FPU
      rundb = overdb + atcoef * (rundb - overdb);
      runratio = averatio + ratatcoef * (runratio - averatio);
#else
      rundb = fmaf(atcoef, rundb - overdb, overdb);
      runratio = fmaf(ratatcoef, runratio - averatio, averatio);
#endif

    } else {

#ifndef _HAS_FPU
      rundb = overdb + relcoef * (rundb - overdb);
      runratio = averatio + ratrelcoef * (runratio - averatio);
#else
      rundb = fmaf(relcoef, rundb - overdb, overdb);
      runratio = fmaf(rratrerlcoef, runratio - averatio, averatio);
#endif

    }

    overdb = rundb;
    averatio = runratio;

    if (allin) {
      cratio = 12 + averatio;
    } else {
      cratio = ratio;
    }

    gr = -overdb * (cratio - 1) / cratio;
    grv = fastExp(gr * DB_TO_LOG);

#ifndef _HAS_FPU
    runmax = maxover + relcoef * (runmax - maxover);  // highest peak for setting att/rel decays in reltime
#else
    runmax = fmaf(relcoef, runmax - maxover, maxover);
#endif

    maxover = runmax;
    spl *= grv * makeupv * mix;
    spl += ospl * (1 - mix);

    outBlock->data[i] = spl;
  }

  // send the block and release the memory
  transmit(outBlock);

  // need to also release the input block because the library uses reference counting...
  release(inBlock);
  release(outBlock);

}

void AudioEffectFetCompressor::setThresholdDb(float thresholdDb) {
  __disable_irq();
  thresh = thresholdDb;
  setThresholdParams(softknee, thresh);
  __enable_irq();
}

void AudioEffectFetCompressor::setSoftKnee(bool softknee) {
  __disable_irq();
  this->softknee = softknee;
  setThresholdParams(softknee, thresh);
  __enable_irq();
}

void AudioEffectFetCompressor::setThresholdParams(bool softknee, float thresh) {
  float cthresh = (softknee ? (thresh - 3) : thresh);
  cthreshv = exp(cthresh * DB_TO_LOG);
  cthreshvRecip = 1 / cthreshv;
}

void AudioEffectFetCompressor::setRatioMode(RatioMode mode) {
  __disable_irq();
  ratioMode = mode;
  rpos = ratioMode;
  capsc = LOG_TO_DB;
  if (rpos > 4) {
    rpos -= 5;
  } else  {
    capsc *= BLOWN_CAP_SCALAR;
  }
  allin = false;
  switch (rpos) {
    case 0:
      ratio = 4;
      break;
    case 1:
      ratio = 8;
      break;
    case 2:
      ratio = 12;
      break;
    case 3:
      ratio = 20;
      break;
    case 4:
      allin = true;
      ratio = 20;
      break;
  }
  cratio = ratio;
  __enable_irq();
}

void AudioEffectFetCompressor::setGainDb(float gain) {
  __disable_irq();
  makeupv = exp((gain + autogain) * DB_TO_LOG);
  __enable_irq();
}

void AudioEffectFetCompressor::setAttackTime(float uSec) {
  __disable_irq();
  float attime = uSec / 1000000;
  atcoef = exp(-1 / (attime * sampleRate));
  __enable_irq();
}

void AudioEffectFetCompressor::setReleaseTime(float mSec) {
  __disable_irq();
  float reltime = mSec / 1000;
  relcoef = exp(-1 / (reltime * sampleRate));
  __enable_irq();
}

void AudioEffectFetCompressor::setMix(float percent) {
  __disable_irq();
  mix = percent * 0.01;
  __enable_irq();
}
