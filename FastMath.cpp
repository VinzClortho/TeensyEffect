#include "FastMath.h"

/*
   - borrowed from https://varietyofsound.wordpress.com/2011/02/14/efficient-tanh-computation-using-lamberts-continued-fraction/
   - just 13 multiply/adds plus a division
   - input/output range: -1.0 to 1.0
   - adds odd interval harmonics
*/
float fastTanh(float x) {
  float x2 = x * x;
  float a = (((x2 + 378) * x2 + 17325) * x2 + 135135) * x;
  float b = ((28 * x2 + 3150) * x2 + 62370) * x2 + 135135;
  return a / b;
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
*/

float addEvenOrderHarmonics(float x) {
  float x2 = x * x;
  float x4 = x2 * x2;
  float x8 = x4 * x4;
  return x + x2 + x4 + x8;
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
float saturation(float y0, float y2, int antiAliasSteps, float drive) {
  float sum = 0.0;
  float step;

  // TODO: is a swtich statement really more performant than a single divide?
  switch (antiAliasSteps) {
    default:
    case 1:
      step = AA_STEP_1;
      break;
    case 2:
      step = AA_STEP_2;
      break;
    case 3:
      step = AA_STEP_3;
      break;
    case 4:
      step = AA_STEP_4;
      break;
    case 5:
      step = AA_STEP_5;
      break;
    case 6:
      step = AA_STEP_6;
      break;
    case 7:
      step = AA_STEP_7;
      break;
    case 8:
      step = AA_STEP_8;
      break;
  }

  float m = (y2 - y0);

  for (float x = 0.0; x < 1.0; x += step) {
    sum += fastTanh(drive * addEvenOrderHarmonics(m * x + y0));
  }

  return sum * step;
}
