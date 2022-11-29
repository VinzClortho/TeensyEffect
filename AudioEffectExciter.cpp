#include "AudioEffectExciter.h"
#include "FastMath.h"

void AudioEffectExciter::init(float sampleRate) {
  this->sampleRate = sampleRate;

  // set defaults
  setFrequency(1200.0);
  setClipBoostDb(6.0);
  setHarmonicsPercent(50.0);
  setMixBackDb(-6.0);
}

void AudioEffectExciter::update(void) {
  // work memory
  audio_block_t *inBlock;
  audio_block_t *outBlock;

  inBlock = receiveReadOnly();
  outBlock = allocate();

  if (inBlock == NULL || outBlock == NULL) return;

  float clipTemp;

  // do the exciting stuff
  for (int i = 0; i < AUDIO_BLOCK_SAMPLES; ++i) {
    spl = inBlock->data[i] * INT_TO_FLOAT;
    s = spl;
    s -=  (tmpONE = a0 * s - b1 * tmpONE + C_DENORM);
    clipTemp = s * clipBoost;
    if (clipTemp < -1) s = -1;
    if (clipTemp > 1) s = 1;
    s = fooPlusOne * s / (1 + foo * abs(spl));
    s -=  (tmpTWO = a0 * s - b1 * tmpTWO + C_DENORM);
    spl += s * mixBack;
    outBlock->data[i] = spl * FLOAT_TO_INT;
  }

  // send the block and release the memory
  transmit(outBlock);

  // need to also release the input block because the library uses reference counting...
  release(inBlock);
  release(outBlock);
}

void AudioEffectExciter::setClipBoostDb(float clipBoostDb) {
  __disable_irq();
  clipBoost = exp(clipBoostDb / C_AMP_DB);
  __enable_irq();
}

void AudioEffectExciter::setMixBackDb(float mixBackDb) {
  __disable_irq();
  mixBack = exp(mixBackDb / C_AMP_DB);
  __enable_irq();
}

void AudioEffectExciter::setHarmonicsPercent(float harmonicsPercent) {
  __disable_irq();
  hdistr = min(harmonicsPercent / 100, .9);
  foo = 2 * hdistr / (1 - hdistr);
  fooPlusOne = foo + 1;
  __enable_irq();
}

void AudioEffectExciter::setFrequency(float frequency) {
  __disable_irq();
  freq = min(frequency, sampleRate);
  x = exp(-2.0 * PI * freq / sampleRate);
  a0 = 1.0 - x;
  b1 = -x;
  __enable_irq();
}
