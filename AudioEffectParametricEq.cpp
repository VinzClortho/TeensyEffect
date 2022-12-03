#include <Arduino.h>
#include <AudioStream.h>
#include "AudioEffectParametricEq.h"
#include "FastMath.h"

void AudioEffectParametricEq::init(float sampleRate) {

  this->sampleRate = sampleRate;
  this->maxFrequency = sampleRate / 2;

  setHpfFreq(hpfFreq);

  __disable_irq();
  setLowParams(this->lowFreq, this->lowQ, this->lowGain);
  setLowMidParams(this->lowMidFreq, this->lowMidQ, this->lowMidGain);
  setLowParams(this->highMidFreq, this->highMidQ, this->highMidGain);
  setLowParams(this->highFreq, this->highQ, this->highGain);
  __enable_irq();

  setLpfFreq(lpfFreq);
  setOutputGain(0);
}

void AudioEffectParametricEq::update(void) {

  // work memory
  audio_block_t *inBlock;
  audio_block_t *outBlock;

  inBlock = receiveReadOnly();
  outBlock = allocate();

  if (inBlock == NULL || outBlock == NULL) return;

  float spl, ospl;

  // get class variables into local variable for better stack usage
  float b00 = this->b00;
  float b10 = this->b10;
  float b20 = this->b20;
  float a10 = this->a10;
  float a20 = this->a20;

  float b01 = this->b01;
  float b11 = this->b11;
  float b21 = this->b21;
  float a11 = this->a11;
  float a21 = this->a21;

  float b03 = this->b03;
  float b13 = this->b13;
  float b23 = this->b23;
  float a13 = this->a13;
  float a23 = this->a23;

  float b05 = this->b05;
  float b15 = this->b15;
  float b25 = this->b25;
  float a15 = this->a15;
  float a25 = this->a25;

  float b07 = this->b07;
  float b17 = this->b17;
  float b27 = this->b27;
  float a17 = this->a17;
  float a27 = this->a27;

  float b09 = this->b09;
  float b19 = this->b19;
  float b29 = this->b29;
  float a19 = this->a19;
  float a29 = this->a29;

  float _x10 = this->_x10;
  float x20 = this->x20;
  float y10 = this->y10;
  float y20 = this->y20;

  float x11 = this->x11;
  float x21 = this->x21;
  float y11 = this->y11;
  float y21 = this->y21;

  float x13 = this->x13;
  float x23 = this->x23;
  float y13 = this->y13;
  float y23 = this->y23;

  float x15 = this->x15;
  float _x25 = this->_x25;
  float y15 = this->y15;
  float y25 = this->y25;

  float x17 = this->x17;
  float x27 = this->x27;
  float y17 = this->y17;
  float y27 = this->y27;

  float x19 = this->x19;
  float x29 = this->x29;
  float y19 = this->y19;
  float y29 = this->y29;

  // do the EQ'ing
  for (int i = 0; i != AUDIO_BLOCK_SAMPLES; ++i) {

    spl = inBlock->data[i];

    // HPF
    if (fastNonZero(hpfFreq)) {
      ospl = spl;
      spl = b00 * spl + b10 * _x10 + b20 * x20 - a10 * y10 - a20 * y20;
      x20 = _x10;
      _x10 = ospl;
      y20 = y10;
      y10 = fastAbs(spl) < C_DENORM ? 0 : spl;
    }

    spl += C_DC_ADD;

    // LOW
    if (fastNonZero(lowFreq) && fastNonZero(lowGain)) {
      ospl = spl;
      spl = b01 * spl + b11 * x11 + b21 * x21 - a11 * y11 - a21 * y21;
      x21 = x11;
      x11 = ospl;
      y21 = y11;
      y11 = spl;
    }

    // LOW-MID
    if (fastNonZero(lowMidFreq) && fastNonZero(lowMidGain)) {
      ospl = spl;
      spl = b03 * spl + b13 * x13 + b23 * x23 - a13 * y13 - a23 * y23;
      x23 = x13;
      x13 = ospl;
      y23 = y13;
      y13 = spl;
    }

    // HIGH-MID
    if (fastNonZero(highMidFreq) && fastNonZero(highMidGain)) {
      ospl = spl;
      spl = b05 * spl + b15 * x15 + b25 * _x25 - a15 * y15 - a25 * y25;
      _x25 = x15;
      x15 = ospl;
      y25 = y15;
      y15 = spl;
    }

    // HIGH
    if (fastNonZero(highFreq) && fastNonZero(highGain)) {
      ospl = spl;
      spl = b07 * spl + b17 * x17 + b27 * x27 - a17 * y17 - a27 * y27;
      x27 = x17;
      x17 = ospl;
      y27 = y17;
      y17 = spl;
    }

    // LPF
    if (lpfFreq < maxFrequency) {
      ospl = spl;
      spl = b09 * spl + b19 * x19 + b29 * x29 - a19 * y19 - a29 * y29;
      x29 = x19;
      x19 = ospl;
      y29 = y19;
      y19 = spl;
    }

    outBlock->data[i] = spl * outGain;
  }

  // send the block and release the memory
  transmit(outBlock);

  // need to also release the input block because the library uses reference counting...
  release(inBlock);
  release(outBlock);

  // copy back to class state
  this->_x10 = _x10;
  this->x20 = x20;
  this->y10 = y10;
  this->y20 = y20;

  this->x11 = x11;
  this->x21 = x21;
  this->y11 = y11;
  this->y21 = y21;

  this->x13 = x13;
  this->x23 = x23;
  this->y13 = y13;
  this->y23 = y23;

  this->x15 = x15;
  this->_x25 = _x25;
  this->y15 = y15;
  this->y25 = y25;

  this->x17 = x17;
  this->x27 = x27;
  this->y17 = y17;
  this->y27 = y27;

  this->x19 = x19;
  this->x29 = x29;
  this->y19 = y19;
  this->y29 = y29;

}

void AudioEffectParametricEq::setHpfFreq(float freq) {
  __disable_irq();
  this->hpfFreq = freq;
  a0 = 1;
  s0 = 1;
  q0 = 1 / (sqrt((a0 + 1 / a0) * (1 / s0 - 1) + 2));
  w00 = 2 * PI * freq / sampleRate;
  cosw00 = cos(w00);
  sinw00 = sin(w00);
  alpha0 = sinw00 / (2 * q0);

  b00 = (1 + cosw00) / 2;
  b10 = -(1 + cosw00);
  b20 = (1 + cosw00) / 2;
  a00 = 1 / (1 + alpha0);
  a10 = -2 * cosw00;
  a20 = 1 - alpha0;
  b00 *= a00;
  b10 *= a00;
  b20 *= a00;
  a10 *= a00;
  a20 *= a00;
  __enable_irq();
}

void AudioEffectParametricEq::setLowFreq(float freq) {
  __disable_irq();
  this->lowFreq = freq;
  setLowParams(this->lowFreq, this->lowQ, this->lowGain);
  __enable_irq();
}

void AudioEffectParametricEq::setLowQ(float q) {
  __disable_irq();
  this->lowQ = q;
  setLowParams(this->lowFreq, this->lowQ, this->lowGain);
  __enable_irq();
}

void AudioEffectParametricEq::setLowGain(float gain) {
  __disable_irq();
  this->lowGain = gain;
  setLowParams(this->lowFreq, this->lowQ, this->lowGain);
  __enable_irq();
}

void AudioEffectParametricEq::setLowParams(float freq, float q, float gain) {
  a1 = pow(10, (gain / 40));
  q1 = q;
  w01 = 2 * PI * freq / sampleRate;
  cosw01 = cos(w01);
  sinw01 = sin(w01);
  alpha1 = sinw01 / (2 * q1);

  b01 = 1 + alpha1 * a1;
  b11 = -2 * cosw01;
  b21 = 1 - alpha1 * a1;
  a01 = 1 / (1 + alpha1 / a1);
  a11 = -2 * cosw01;
  a21 = 1 - alpha1 / a1;
  b01 *= a01;
  b11 *= a01;
  b21 *= a01;
  a11 *= a01;
  a21 *= a01;
}

void AudioEffectParametricEq::setLowMidFreq(float freq) {
  __disable_irq();
  this->lowMidFreq = freq;
  setLowMidParams(this->lowMidFreq, this->lowMidQ, this->lowMidGain);
  __enable_irq();
}

void AudioEffectParametricEq::setLowMidQ(float q) {
  __disable_irq();
  this->lowMidQ = q;
  setLowMidParams(this->lowMidFreq, this->lowMidQ, this->lowMidGain);
  __enable_irq();
}

void AudioEffectParametricEq::setLowMidGain(float gain) {
  __disable_irq();
  this->lowMidGain = gain;
  setLowMidParams(this->lowMidFreq, this->lowMidQ, this->lowMidGain);
  __enable_irq();
}

void AudioEffectParametricEq::setLowMidParams(float freq, float q, float gain) {
  a3 = pow(10, (gain / 40));
  q3 = q;
  w03 = 2 * PI * freq / sampleRate;
  cosw03 = cos(w03);
  sinw03 = sin(w03);
  alpha3 = sinw03 / (2 * q3);

  b03 = 1 + alpha3 * a3;
  b13 = -2 * cosw03;
  b23 = 1 - alpha3 * a3;
  a03 = 1 / (1 + alpha3 / a3);
  a13 = -2 * cosw03;
  a23 = 1 - alpha3 / a3;
  b03 *= a03;
  b13 *= a03;
  b23 *= a03;
  a13 *= a03;
  a23 *= a03;
}

void AudioEffectParametricEq::setHighMidFreq(float freq) {
  __disable_irq();
  this->highMidFreq = freq;
  setHighMidParams(this->highMidFreq, this->highMidQ, this->highMidGain);
  __enable_irq();
}

void AudioEffectParametricEq::setHighMidQ(float q) {
  __disable_irq();
  this->highMidQ = q;
  setHighMidParams(this->highMidFreq, this->highMidQ, this->highMidGain);
  __enable_irq();
}

void AudioEffectParametricEq::setHighMidGain(float gain) {
  __disable_irq();
  this->highMidGain = gain;
  setHighMidParams(this->highMidFreq, this->highMidQ, this->highMidGain);
  __enable_irq();
}

void AudioEffectParametricEq::setHighMidParams(float freq, float q, float gain) {
  a5 = pow(10, (gain / 40));
  q5 = q;
  w05 = 2 * PI * freq / sampleRate;
  cosw05 = cos(w05);
  sinw05 = sin(w05);
  alpha5 = sinw05 / (2 * q5);

  b05 = 1 + alpha5 * a5;
  b15 = -2 * cosw05;
  b25 = 1 - alpha5 * a5;
  a05 = 1 / (1 + alpha5 / a5);
  a15 = -2 * cosw05;
  a25 = 1 - alpha5 / a5;
  b05 *= a05;
  b15 *= a05;
  b25 *= a05;
  a15 *= a05;
  a25 *= a05;
}

void AudioEffectParametricEq::setHighFreq(float freq) {
  __disable_irq();
  this->highFreq = freq;
  setHighParams(this->highFreq, this->highQ, this->highGain);
  __enable_irq();
}

void AudioEffectParametricEq::setHighQ(float q) {
  __disable_irq();
  this->highQ = q;
  setHighParams(this->highFreq, this->highQ, this->highGain);
  __enable_irq();
}

void AudioEffectParametricEq::setHighGain(float gain) {
  __disable_irq();
  this->highGain = gain;
  setHighParams(this->highFreq, this->highQ, this->highGain);
  __enable_irq();
}

void AudioEffectParametricEq::setHighParams(float freq, float q, float gain) {
  a7 = pow(10, (gain / 40));
  q7 = q;
  w07 = 2 * PI * freq / sampleRate;
  cosw07 = cos(w07);
  sinw07 = sin(w07);
  alpha7 = sinw07 / (2 * q7);

  b07 = 1 + alpha7 * a7;
  b17 = -2 * cosw07;
  b27 = 1 - alpha7 * a7;
  a07 = 1 / (1 + alpha7 / a7);
  a17 = -2 * cosw07;
  a27 = 1 - alpha7 / a7;
  b07 *= a07;
  b17 *= a07;
  b27 *= a07;
  a17 *= a07;
  a27 *= a07;
}

void AudioEffectParametricEq::setLpfFreq(float freq) {
  __disable_irq();
  this->lpfFreq = freq;
  a9 = 1;
  s9 = 2;
  q9 = 1 / (sqrt((a9 + 1 / a9) * (1 / s9 - 1) + 2));
  w09 = 2 * PI * freq / sampleRate;
  cosw09 = cos(w09);
  sinw09 = sin(w09);
  alpha9 = sinw09 / (2 * q9);

  b09 = (1 - cosw09) / 2;
  b19 = (1 - cosw09);
  b29 = (1 - cosw09) / 2;
  a09 = 1 / (1 + alpha9);
  a19 = -2 * cosw09;
  a29 = 1 - alpha9;
  b09 *= a09;
  b19 *= a09;
  b29 *= a09;
  a19 *= a09;
  a29 *= a09;
  __enable_irq();
}

void AudioEffectParametricEq::setOutputGain(float gain) {
  __disable_irq();
  this->outGain = pow(10, gain / 20);
  __enable_irq();
}
