#ifndef _AUDIO_EFFECT_OPTICAL_COMP_H
#define _AUDIO_EFFECT_OPTICAL_COMP_H

#include <Arduino.h>
#include <AudioStream.h>
#include "FastMath.h"

#define OPT_COMP_RATIO 20
#define OPT_COMP_RATIO_MINUS_ONE OPT_COMP_RATIO-1

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
    float getGainReduction() ;

  private:
    audio_block_t *inputQueueArray[1];

    float sampleRate;

    float threshvRecip;
    float biasRecip;
    float makeupv;
    float capsc;
    float atcoef, relcoef, rmscoef;
    float runave = 0.0f, rundb = 0.0f;
    float gr;
};

#endif /* _AUDIO_EFFECT_OPTICAL_COMP_H */
