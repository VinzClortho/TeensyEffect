#ifndef _AUDIO_EFFECT_TUBE_SATURATION_H
#define _AUDIO_EFFECT_TUBE_SATURATION_H

#include <Arduino.h>
#include <AudioStream.h>

#define ANTI_ALIASING_STEPS 4
#define NO_LPF_HISTORY_YET -12345

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
    void setLpfFrequency(float freq);

  private:
    audio_block_t *inputQueueArray[1];

    float addEvenOrderHarmonics(float x);
    float saturation(float y0, float y2, int antiAliasSteps, float drive);

    float sampleRate;

    float drive;
    float makeupGain;

    float inSpl, spl;
    float lastSpl;

    float alpha;
    float lastLpfSpl = NO_LPF_HISTORY_YET;
    float satSpl, lastSatSpl;

};

#endif /* _AUDIO_EFFECT_TUBE_SATURATION_H */
