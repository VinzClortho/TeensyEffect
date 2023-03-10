#ifndef _AUDIO_EFFECT_FET_COMP_H
#define _AUDIO_EFFECT_FET_COMP_H

#include <Arduino.h>
#include <AudioStream.h>
#include "FastMath.h"

enum RatioMode {
  BlownCap4, BlownCap8, BlownCap12, BlownCap20, BlownCapAll, Clean4, Clean8, Clean12, Clean20, CleanAll
};

class AudioEffectFetCompressor : public AudioStream
{
  public:
    AudioEffectFetCompressor() : AudioStream(1, inputQueueArray) {
      // any extra initialization
    }
    void init(float sampleRate);
    virtual void update(void);

    void setThresholdDb(float thresholdDb);
    void setRatioMode(RatioMode mode);
    void setSoftKnee(bool softknee);
    void setGainDb(float gain);
    void setAttackTimeUs(float uSec);
    void setReleaseTimeMs(float mSec);
    void setMix(float percent);

  private:
    audio_block_t *inputQueueArray[1];

    float sampleRate;

    void setThresholdParams(bool softknee, float thresh);

    // from controls
    RatioMode ratioMode;
    bool softknee;
    float thresh;

    float ratio;
    float rundb;
    float ratatcoef;
    float ratrelcoef;
    float atcoef;
    float relcoef;
    float mix, oneMinusMix;

    float capsc;
    bool allin;
    float cthreshv, cthreshvRecip;
    float makeupv;
    float runave, rmscoef = 1, runmax, maxover;
    float averatio, runratio;

};

#endif /* _AUDIO_EFFECT_FET_COMP_H */
