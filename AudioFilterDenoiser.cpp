#include "AudioFilterDenoiser.h"

void AudioFilterDenoiser::init(float sampleRate) {
  this->sampleRate = sampleRate;

  // defaults

}

#define STEP 0.25;

void AudioFilterDenoiser::update(void) {
  // work memory
  audio_block_t *inBlock;
  audio_block_t *outBlock;

  inBlock = receiveReadOnly();
  outBlock = allocate();

  if (inBlock == NULL || outBlock == NULL) return;

  // do the saturation stuff
  for (int i = 0; i != AUDIO_BLOCK_SAMPLES; ) {

    // store currrent i
    int ii = i;

    // calculate mean using bit-shifting and addition
    int iSpl = inBlock->data[i++] >> 2;
    iSpl += inBlock->data[i++] >> 2;
    iSpl += inBlock->data[i++] >> 2;
    iSpl += inBlock->data[i++] >> 2;

    float spl = (float)iSpl * INT_TO_FLOAT;

    // TODO!
    float m = spl - lastSpl;
    float x = 0.0f;

    spl = m * x + lastSpl;
    outBlock->data[ii++] = (int)(spl * FLOAT_TO_INT);
    x += STEP;

    spl = m * x + lastSpl;
    outBlock->data[ii++] = (int)(spl * FLOAT_TO_INT);
    x += STEP;

    spl = m * x + lastSpl;
    outBlock->data[ii++] = (int)(spl * FLOAT_TO_INT);
    x += STEP;

    spl = m * x + lastSpl;
    outBlock->data[ii++] = (int)(spl * FLOAT_TO_INT);
    x += STEP;

    lastSpl = spl;

  }

  // send the block and release the memory
  transmit(outBlock);

  // need to also release the input block because the library uses reference counting...
  release(inBlock);
  release(outBlock);
}
