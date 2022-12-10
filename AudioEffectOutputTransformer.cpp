#include "AudioEffectOutputTransformer.h"

void AudioEffectOutputTransformer::update(void) {
  // work memory
  audio_block_t *inBlock;
  audio_block_t *outBlock;

  inBlock = receiveReadOnly();
  outBlock = allocate();

  if (inBlock == NULL || outBlock == NULL) return;

  // do the saturation stuff
  for (int i = 0; i < AUDIO_BLOCK_SAMPLES; ++i) {
    float spl = (float)inBlock->data[i] * INT_TO_FLOAT;
    spl = fastTanh(drive * spl);
    outBlock->data[i] = (int)(spl * FLOAT_TO_INT);
  }

  // send the block and release the memory
  transmit(outBlock);

  // need to also release the input block because the library uses reference counting...
  release(inBlock);
  release(outBlock);
}

void AudioEffectOutputTransformer::setDrive(float drive) {
  __disable_irq();
  this->drive = drive;
  __enable_irq();
}
