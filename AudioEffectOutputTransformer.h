#ifndef _AUDIO_EFFECT_OUTPUT_TRANSFORMER_H
#define _AUDIO_EFFECT_OUTPUT_TRANSFORMER_H

#include <Arduino.h>
#include <AudioStream.h>

#include "FastMath.h"

class AudioEffectOutputTransformer : public AudioStream
{
  public:
    AudioEffectOutputTransformer() : AudioStream(1, inputQueueArray) {
      // any extra initialization
    }
    virtual void update(void);

  private:
    audio_block_t *inputQueueArray[1];
};

#endif /* _AUDIO_EFFECT_OUTPUT_TRANSFORMER_H */
