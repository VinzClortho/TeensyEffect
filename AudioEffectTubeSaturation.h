#ifndef _AUDIO_EFFECT_TUBE_SATURATION_H
#define _AUDIO_EFFECT_TUBE_SATURATION_H

#include <Arduino.h>
#include <AudioStream.h>

#define ANTI_ALIASING_STEPS 4

class AudioEffectTubeSaturation : public AudioStream
{
  public:
    AudioEffectTubeSaturation() : AudioStream(1, inputQueueArray) {
      // any extra initialization
    }
    void init(float sampleRate);
    virtual void update(void);

    void setDrive(float drive);
    void setMakeupGainDb(float gain);

  private:
    audio_block_t *inputQueueArray[1];

    float sampleRate;

    float drive;
    float makeupGain;

    float inSpl, spl;
    float lastSpl;
};

#endif /* _AUDIO_EFFECT_TUBE_SATURATION_H */
