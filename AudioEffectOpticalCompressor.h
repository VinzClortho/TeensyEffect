#ifndef _AUDIO_EFFECT_OPTICAL_COMP_H
#define _AUDIO_EFFECT_OPTICAL_COMP_H

#include <Arduino.h>
#include <AudioStream.h>
#include "FastMath.h"

class AudioEffectOpticalCompressor : public AudioStream
{
  public:
    AudioEffectOpticalCompressor() : AudioStream(1, inputQueueArray) {
      // any extra initialization
    }
    void init(float sampleRate);
    virtual void update(void);

    void setThresholdDb(float thresh);
    void setBias(float bias);
    void setMakeupGainDb(float gain);
    void setBlownCapacitor(bool blownCap);
    void setTimeConstant(int tc);
    void setRmsWindowUs(int windowUs);

  private:
    audio_block_t *inputQueueArray[1];

    float sampleRate;
    
    float threshvRecip;
    float biasRecip;
    float makeupv;
    float capsc;
    float atcoef, relcoef, rmscoef;
    float runave, rundb = 0.0f;
    float ratio = 20.0f;
};

#endif /* _AUDIO_EFFECT_OPTICAL_COMP_H */
