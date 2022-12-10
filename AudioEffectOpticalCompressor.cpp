#include "AudioEffectOpticalCompressor.h"

void AudioEffectOpticalCompressor::init(float sampleRate) {
  this->sampleRate = sampleRate;

  // defaults
  setThresholdDb(-3.0);
  setBias(70.0);
  setMakeupGainDb(0.0);
  setBlownCapacitor(true);
  setTimeConstant(1);
  setRmsWindowUs(100);
}

void AudioEffectOpticalCompressor::update(void) {

  // work memory
  audio_block_t *inBlock;
  audio_block_t *outBlock;

  inBlock = receiveReadOnly();
  outBlock = allocate();

  if (inBlock == NULL || outBlock == NULL) return;

  // copy in class state
  float runave = this->runave;
  float rmscoef = this->rmscoef;
  float capsc = this->capsc;
  float threshvRecip = this->threshvRecip;
  float rundb = this->rundb;
  float atcoef = this->atcoef;
  float relcoef = this->relcoef;
  float biasRecip = this->biasRecip;
  float makeupv = this->makeupv;

  // do the compressing
  for (int i = 0; i < AUDIO_BLOCK_SAMPLES; ++i) {

    float spl = (float)inBlock->data[i] * INT_TO_FLOAT;
    float maxspl = spl * spl;

    runave = maxspl + rmscoef * (runave - maxspl);

    float det = fastSqrt(max(0, runave));
    float overdb = capsc * fastLog(det * threshvRecip);
    overdb = max(0, overdb);

    float dbDelta = rundb - overdb;

    rundb = overdb;
    // dbDelta will be negative if overdb is greater than rundb, so we're in the attack phase.  Otherwise, we're in the release phase.
    rundb += (dbDelta < 0.0f ? atcoef : relcoef) * dbDelta;

    overdb = max(rundb, 0);

    float cratio = OPT_COMP_RATIO_MINUS_ONE * fastSqrt(overdb * biasRecip);
    gr = -overdb * cratio  / (cratio + 1);
    float grv = fastExp(gr * DB_TO_LOG);

    spl *= grv * makeupv;

    outBlock->data[i] = (int)(spl * FLOAT_TO_INT);
  }

  // send the block and release the memory
  transmit(outBlock);

  // copy back to class state
  this->runave = runave;
  this->rundb = rundb;

  // need to also release the input block because the library uses reference counting...
  release(inBlock);
  release(outBlock);

}

float AudioEffectOpticalCompressor::getGainReduction() {
  return gr;
}

void AudioEffectOpticalCompressor::setThresholdDb(float thresh) {
  __disable_irq();
  threshvRecip = 1.0 / exp(thresh * DB_TO_LOG);
  Serial.print("threshvRecip: ");
  Serial.println(threshvRecip);

  __enable_irq();
}

void AudioEffectOpticalCompressor::setBias(float bias) {
  __disable_irq();
  // Always have a slight bias. This simpifies later logic.
  if (bias < 0.1) bias = 0.1;
  bias *= 0.8;
  this->biasRecip = 1.0f / bias;
  Serial.print("biasRecip: ");
  Serial.println(biasRecip);
  __enable_irq();
}

void AudioEffectOpticalCompressor::setMakeupGainDb(float gain) {
  __disable_irq();
  makeupv = exp(gain * DB_TO_LOG);
  Serial.print("makeupv: ");
  Serial.println(makeupv);
  __enable_irq();
}

void AudioEffectOpticalCompressor::setBlownCapacitor(bool blownCap) {
  __disable_irq();
  this->capsc = blownCap ? LOG_TO_DB : LOG_TO_DB * BLOWN_CAP_SCALAR;
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

  Serial.print("atcoef: ");
  Serial.println(atcoef);

  Serial.print("relcoef: ");
  Serial.println(relcoef);

  __enable_irq();
}

void AudioEffectOpticalCompressor::setRmsWindowUs(int windowUs) {
  __disable_irq();
  float rmstime = (float)windowUs * 0.000001;
  Serial.print("rmstime: ");
  Serial.println(rmstime);
  rmscoef = exp(-1 / (rmstime * sampleRate));
  Serial.print("rmscoef: ");
  Serial.println(rmscoef);
  __enable_irq();
}
