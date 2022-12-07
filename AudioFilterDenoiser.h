#ifndef _AUDIO_FILTER_DENOISER_H
#define _AUDIO_FILTER_DENOISER_H

#include <Arduino.h>
#include <AudioStream.h>

#include "FastMath.h"

class AudioFilterDenoiser : public AudioStream
{
  public:
    AudioFilterDenoiser() : AudioStream(1, inputQueueArray) {
      // any extra initialization
    }
    void init(float sampleRate);
    virtual void update(void);

  private:
    audio_block_t *inputQueueArray[1];

    float sampleRate;
    float lastSpl;
};

#endif /* _AUDIO_FILTER_DENOISER_H */
