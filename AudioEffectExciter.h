#ifndef _AUDIO_EFFECT_EXCITER_H
#define _AUDIO_EFFECT_EXCITER_H

#include <Arduino.h>
#include <AudioStream.h>

class AudioEffectExciter : public AudioStream
{
  public:
    AudioEffectExciter() : AudioStream(1, inputQueueArray) {
      // any extra initialization
    }
    void init(float sampleRate);
    virtual void update(void);

    void setClipBoostDb(float clipBoostDb);
    void setMixBackDb(float mixBackDb);
    void setHarmonicsPercent(float harmonicsPercent);
    void setFrequency(float frequency);

  private:
    audio_block_t *inputQueueArray[1];

    float sampleRate;

    float clipBoost;
    float mixBack;
    float hdistr;
    float foo;
    float freq;
    float x;
    float a0;
    float b1;
    float tmpONE, tmpTWO;

    float spl, s;
};

#endif /* _AUDIO_EFFECT_EXCITER_H */
