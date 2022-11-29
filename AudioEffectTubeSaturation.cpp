#include "AudioEffectTubeSaturation.h"
#include "FastMath.h"

void AudioEffectTubeSaturation::init(float sampleRate) {
  this->sampleRate = sampleRate;

  // defaults
  setDrive(0.5);
  setMakeupGainDb(0.0);
}

void AudioEffectTubeSaturation::update(void) {
  // work memory
  audio_block_t *inBlock;
  audio_block_t *outBlock;

  inBlock = receiveReadOnly();
  outBlock = allocate();

  if (inBlock == NULL || outBlock == NULL) return;

  // do the saturation stuff
  for (int i = 0; i < AUDIO_BLOCK_SAMPLES; ++i) {
    inSpl = inBlock->data[i] * INT_TO_FLOAT;

    // saturation
    spl = saturation(lastSpl, inSpl, ANTI_ALIASING_STEPS, drive);
    lastSpl = inSpl;

    // TODO: LPF

    spl *= makeupGain;

    outBlock->data[i] = spl * FLOAT_TO_INT;
  }

  // send the block and release the memory
  transmit(outBlock);

  // need to also release the input block because the library uses reference counting...
  release(inBlock);
  release(outBlock);
}

void AudioEffectTubeSaturation::setDrive(float drive) {
  __disable_irq();
  this->drive = drive;
  __enable_irq();
}

void AudioEffectTubeSaturation::setMakeupGainDb(float gain) {
  __disable_irq();
  makeupGain = exp(gain * DB_TO_LOG);
  __enable_irq();
}
