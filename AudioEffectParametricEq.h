#ifndef _AUDIO_EFFECT_PARA_EQ_H
#define _AUDIO_EFFECT_PARA_EQ_H

#include "AudioStream.h"

class AudioEffectParametricEq : public AudioStream {
  public:
    AudioEffectParametricEq() : AudioStream(1, inputQueueArray) {
      // any extra initialization
    }
    void init(float sampleRate);

    virtual void update(void);

    void setHpfFreq(float freq);
    void setLowFreq(float freq);
    void setLowQ(float q);
    void setLowGain(float gain);
    void setLowMidFreq(float freq);
    void setLowMidQ(float q);
    void setLowMidGain(float gain);
    void setHighMidFreq(float freq);
    void setHighMidQ(float q);
    void setHighMidGain(float gain);
    void setHighFreq(float freq);
    void setHighQ(float q);
    void setHighGain(float gain);
    void setLpfFreq(float freq);
    void setOutputGain(float gain);

  private:
    audio_block_t *inputQueueArray[1];

    void setLowParams(float freq, float q, float gain);
    void setLowMidParams(float freq, float q, float gain);
    void setHighMidParams(float freq, float q, float gain);
    void setHighParams(float freq, float q, float gain);

    float sampleRate;
    float maxFrequency;

    // with defaults
    float hpfFreq = 30;
    float lowFreq = 315;
    float lowQ = 1;
    float lowGain = 1;
    float lowMidFreq = 800;
    float lowMidQ = 2;
    float lowMidGain = 3;
    float highMidFreq = 2500;
    float highMidQ = 1;
    float highMidGain = 1;
    float highFreq = 9000;
    float highQ = 0.5;
    float highGain = -3;
    float lpfFreq = 8000;
    float outGain = 1;

    float a0;
    float s0;
    float q0;
    float w00;
    float cosw00;
    float sinw00;
    float alpha0;

    float b00;
    float b10;
    float b20;
    float a00;
    float a10;
    float a20;

    float a1;
    float q1;
    float w01;
    float cosw01;
    float sinw01;
    float alpha1;

    float b01;
    float b11;
    float b21;
    float a01;
    float a11;
    float a21;

    float a3;
    float q3;
    float w03;
    float cosw03;
    float sinw03;
    float alpha3;

    float b03;
    float b13;
    float b23;
    float a03;
    float a13;
    float a23;

    float a5;
    float q5;
    float w05;
    float cosw05;
    float sinw05;
    float alpha5;

    float b05;
    float b15;
    float b25;
    float a05;
    float a15;
    float a25;

    float a7;
    float q7;
    float w07;
    float cosw07;
    float sinw07;
    float alpha7;

    float b07;
    float b17;
    float b27;
    float a07;
    float a17;
    float a27;

    float a9;
    float s9;
    float q9;
    float w09;
    float cosw09;
    float sinw09;
    float alpha9;

    float b09;
    float b19;
    float b29;
    float a09;
    float a19;
    float a29;

    float spl, ospl;
    float y10, y20, _x10, x20;
    float y11, y21, x11, x21;
    float y13, y23, x13, x23;
    float y15, y25, x15, _x25;
    float y17, y27, x17, x27;
    float y19, y29, x19, x29;

};

#endif /* _AUDIO_EFFECT_PARA_EQ_H */
