#include "AudioEffectTubeSaturation.h"
#include "FastMath.h"

void AudioEffectTubeSaturation::init(float sampleRate) {
  this->sampleRate = sampleRate;

  // defaults
  setDrive(1.0f);  // max drive for low accuracy mode!
  setMakeupGainDb(0.0);
  setLpfFrequency(5000.0);
}

void AudioEffectTubeSaturation::update(void) {
  // work memory
  audio_block_t *inBlock;
  audio_block_t *outBlock;

  inBlock = receiveReadOnly();
  outBlock = allocate();

  if (inBlock == NULL || outBlock == NULL) return;

  // do the saturation stuff
  for (int i = 0; i != AUDIO_BLOCK_SAMPLES; ++i) {

    inSpl = (float)inBlock->data[i] * INT_TO_FLOAT;

    // saturation
    satSpl = saturation(lastSpl, inSpl, drive);
    lastSpl = inSpl;

    // LPF
    spl = lastSatSpl + alpha * (satSpl - lastSatSpl);

    lastSatSpl = satSpl;

    // Low pass filter
    lastLpfSpl = spl = lastLpfSpl + alpha * (spl - lastLpfSpl);

    spl *= makeupGain;

    outBlock->data[i] = (int)(spl * FLOAT_TO_INT);
  }

  // send the block and release the memory
  transmit(outBlock);

  // need to also release the input block because the library uses reference counting...
  release(inBlock);
  release(outBlock);
}

/*
  - borrowed from https://dsp.stackexchange.com/questions/5959/add-odd-even-harmonics-to-signal

  If you want to add odd harmonics, put your signal through an odd-symmetric transfer function like y = tanh(x) or y = x^3.

  If you want to add only even harmonics, put your signal through a transfer function that's even symmetric plus an identity function,
  to keep the original fundamental. Something like y = x + x^4 or y = x + abs(x). The x + keeps the fundamental that would otherwise be
  destroyed, while the x^4 is even-symmetric and produces only even harmonics (including DC, which you probably want to remove
  afterwards with a high-pass filter).

  - just 6 multiply/adds
  - input range: -1.0 to 1.0
  - output may exceed input range!
  - adds even interval harmonics
  - TODO: how many even orders are enough? The fewer the better as this will generally be getting called from a loop.
  - removed 6th harmonic as it would relate to a perfect 5th tone wise. Intervals 2, 4, and 8 are perfect octaves from the fundamental.

  https://en.wikipedia.org/wiki/Chebyshev_polynomials
*/
#define ROOT_SCALAR 0.8
#define SECOND_SCALAR 0.4
#define FOURTH_SCALAR 0.2

float AudioEffectTubeSaturation::addEvenOrderHarmonics(float x) {
  float x2 = x * x;

  //  return ROOT_SCALAR * x + SECOND_SCALAR * x2;

  //  return x * (ROOT_SCALAR + x * (SECOND_SCALAR + FOURTH_SCALAR * x2));  // basic polynomial

  return x * (4 * x2 * (3 * x2 - 2) + 3); // Chebyshev polynomial of fundamental, 1nd and third harmonics

  // https://www.wolframalpha.com/input?i=x%2B2x%5E2-1%2B6x%5E4-4x%5E2%2B1
  //    return x * (x * (6 * x - 2) + 1); // Chebyshev polynomial of fundamental, 2nd and forth harmonics

}

/**
   Use linear interpolation for anti-aliasing. Using fixed constant steps to avoid divides.
   Generate even order harrmonics then drive through tanh.
   y0 is last input and y2 is current input.
   antiAliasSteps is the oversampling amount:
      - range 1 to 8
      - suggested 2 to 4?
      - if using antiAliasSteps=1, then y0 should be 0
   drive is a value between 0.0 and 1.0
*/
float AudioEffectTubeSaturation::saturation(float y0, float y2, float drive) {

#if OVERSAMPLING < 2
  return fastTanh(drive * addEvenOrderHarmonics(y2));
#endif

#if OVERSAMPLING > 1
  float sum = 0.0;

  // over an x delta of 1, so simple math
  float m = y2 - y0;

#if OVERSAMPLING == 2
  sum += fastTanh(drive * addEvenOrderHarmonics(m * 0.5 + y0));
  sum += fastTanh(drive * addEvenOrderHarmonics(y2));
  return sum * 0.5;

#elif OVERSAMPLING == 4
  float mStep = m * 0.25;
  float w = y0 + mStep;
  sum += fastTanh(drive * addEvenOrderHarmonics(w));
  w += mStep;
  sum += fastTanh(drive * addEvenOrderHarmonics(w));
  w += mStep;
  sum += fastTanh(drive * addEvenOrderHarmonics(w));
  sum += fastTanh(drive * addEvenOrderHarmonics(y2));
  return sum * 0.25;

#elif OVERSAMPLING == 8
  float mStep = m * 0.125;
  float w = y0 + mStep;
  sum += fastTanh(drive * addEvenOrderHarmonics(w));
  w += mStep;
  sum += fastTanh(drive * addEvenOrderHarmonics(w));
  w += mStep;
  sum += fastTanh(drive * addEvenOrderHarmonics(w));
  w += mStep;
  sum += fastTanh(drive * addEvenOrderHarmonics(w));
  w += mStep;
  sum += fastTanh(drive * addEvenOrderHarmonics(w));
  w += mStep;
  sum += fastTanh(drive * addEvenOrderHarmonics(w));
  w += mStep;
  sum += fastTanh(drive * addEvenOrderHarmonics(w));
  sum += fastTanh(drive * addEvenOrderHarmonics(y2));
  return sum * 0.125;

#else
  // just pass through newest value
  return y2;

#endif

#endif  // OVERSAMPLING > 1
}

void AudioEffectTubeSaturation::setDrive(float drive) {
  __disable_irq();
  this->drive = drive;
  __enable_irq();
}

void AudioEffectTubeSaturation::setMakeupGainDb(float gain) {
  __disable_irq();
  makeupGain = exp(gain * DB_TO_LOG);
  __enable_irq();
}

void AudioEffectTubeSaturation::setLpfFrequency(float freq) {
  __disable_irq();
  float RC = 1.0 / (freq * 2 * PI);
  float dt = 1.0 / sampleRate;
  alpha = dt / (RC + dt);
  __enable_irq();
}
