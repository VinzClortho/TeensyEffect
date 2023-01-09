#include "AudioEffectFetCompressor.h"

void AudioEffectFetCompressor::init(float sampleRate) {
  this->sampleRate = sampleRate;

  // set defaults
  setThresholdDb(-6.0);
  setRatioMode(CleanAll);
  setGainDb(0.0);
  setAttackTimeUs(20);
  setReleaseTimeMs(50);
  setMix(100.0);
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

  // copy from class state
  float runave = this->runave;
  float capsc = this->capsc;
  float cthreshvRecip = this->cthreshvRecip;
  float rundb = this->rundb;
  float averatio = this->averatio;
  float runratio = this->runratio;
  float atcoef = this->atcoef;
  float ratatcoef = this->ratatcoef;
  float relcoef = this->relcoef;
  float ratrelcoef = this->ratrelcoef;
  float runmax = this->runmax;
  float maxover = this->maxover;
  float makeupv = this->makeupv;
  float mix = this->mix;
  float oneMinusMix = this->oneMinusMix;

  // do the compressing
  for (int i = 0; i < AUDIO_BLOCK_SAMPLES; ++i) {

    float spl = (float)inBlock->data[i] * INT_TO_FLOAT;
    float ospl = spl;
    float maxspl = spl * spl;

    runave = maxspl + rmscoef * (runave - maxspl);

    float det = fastSqrt(max(0, runave));
    float overdb = max(0, capsc * fastLog(det * cthreshvRecip));

    float dbDelta = rundb - overdb;
    if (dbDelta < -5) averatio = 4;

    float ratioDelta = runratio - averatio;

    // If dbDelta is less than 0, that means that overdb is greater than rundb and we are in the attack phase. Otherwise, we are in the release phase.
    if (dbDelta < 0.0f) {
      rundb = overdb + atcoef * dbDelta;
      runratio = averatio + ratatcoef * ratioDelta;
    } else {
      rundb = overdb + relcoef * dbDelta;
      runratio = averatio + ratrelcoef * ratioDelta;
    }

    overdb = rundb;
    averatio = runratio;

    float cratio = allin ? 12 + averatio : ratio;
    float gr = -overdb * (cratio - 1) / cratio;
    float grv = fastExp(gr * DB_TO_LOG);

    runmax = maxover + relcoef * (runmax - maxover);  // highest peak for setting att/rel decays in reltime

    maxover = runmax;
    spl *= grv * makeupv * mix;
    spl += ospl * oneMinusMix;

    outBlock->data[i] = (int)(spl * FLOAT_TO_INT);
  }

  // send the block and release the memory
  transmit(outBlock);

  // copy back to class state
  this->runave = runave;
  this->rundb = rundb;
  this->averatio = averatio;
  this->runratio = runratio;
  this->runmax = runmax;
  this->maxover = maxover;

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
  cthreshv = fastExp(cthresh * DB_TO_LOG);
  cthreshvRecip = 1 / cthreshv;
}

void AudioEffectFetCompressor::setRatioMode(RatioMode mode) {
  __disable_irq();
  ratioMode = mode;
  int rpos = ratioMode;
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
  __enable_irq();
}

void AudioEffectFetCompressor::setGainDb(float gain) {
  __disable_irq();
  makeupv = fastExp((gain) * DB_TO_LOG);
  __enable_irq();
}

void AudioEffectFetCompressor::setAttackTimeUs(float uSec) {
  __disable_irq();
  float attime = uSec / 1000000;
  atcoef = fastExp(-1 / (attime * sampleRate));
  __enable_irq();
}

void AudioEffectFetCompressor::setReleaseTimeMs(float mSec) {
  __disable_irq();
  float reltime = mSec / 1000;
  relcoef = fastExp(-1 / (reltime * sampleRate));
  __enable_irq();
}

void AudioEffectFetCompressor::setMix(float percent) {
  __disable_irq();
  mix = percent * 0.01;
  oneMinusMix = 1 - mix;
  __enable_irq();
}
