#include <Arduino.h>
#include "FastMath.h"

/////////////////////////////
// Tanh implementation
/////////////////////////////
/*
   - borrowed from https://varietyofsound.wordpress.com/2011/02/14/efficient-tanh-computation-using-lamberts-continued-fraction/
   - just 13 multiply/adds plus a division
   - input/output range: -1.0 to 1.0
   - adds odd interval harmonics
   - maybe just up a big of accuracy from here: https://www.wolframalpha.com/input?i=x%2F%281%2Bx%5E2%2F%283%2Bx%5E2%2F%285%2Bx%5E2%2F7%29%29%29+
*/
float fastTanh(float x) {
  float x2 = x * x;

#ifdef _HIGHER_ACCURACY
#ifdef _HAS_FPU
  float tmp;
#endif

#ifndef _HAS_FPU
  float a = (((x2 + 378) * x2 + 17325) * x2 + 135135) * x;
  float b = ((28 * x2 + 3150) * x2 + 62370) * x2 + 135135;
#else
  tmp = x2 + 378;
  tmp = fmaf(tmp, x2, 17325);
  tmp = fmaf(tmp, x2, 135135);
  float a = tmp * x;
  tmp = fmaf(28, x2, 3150);
  tmp = fmaf(tmp, x2, 62370);
  float b = fmaf(tmp, x2, 135135);
#endif
#else
  // super fast version, only really usable between +-2.5 input
  //  float a = x * (x2 + 15);
  //  float b = 6 * x2 + 15;

  // medium acccuracy version, usable over a much wider range
  float a = 5 * x * (2 * x2 + 21);
  float b = x2 * (x2 + 45) + 105;

#endif

  return a / b;
}

/////////////////////////////
// Sqrt implementation
/////////////////////////////
/**
   https://en.wikipedia.org/wiki/Methods_of_computing_square_roots#Approximations_that_depend_on_the_floating_point_representation

   Fastest so far and averages about 2% error without iteration.
*/
float fastSqrt(const float x) {
  uint32_t i = *(uint32_t*)&x;
  i -= 1 << 23; /* Subtract 2^m. */
  i >>= 1;    /* Divide by 2. */
  i += 1 << 29; /* Add ((b + 1) / 2) * 2^m. */
  i &= 0x7FFFFFFF; /* ensure that sign bit is not set */
  float f = *(float*)&i;

  // this will improve accuracy but up to triple CPU cycles
#ifdef _HIGHER_ACCURACY
  return (f * f + x) / (2.0f * f);
#endif

  return f;   /* Interpret again as float */
}

/////////////////////////////
// Reeciprocal implementation
/////////////////////////////
/**
   https://stackoverflow.com/questions/12227126/division-as-multiply-and-lut-fast-float-division-reciprocal
*/
float fastRecip(const float f) {
  // get a good estimate via bit twiddling
  uint32_t x = *(uint32_t*)&f;
  x = 0x7EF311C2 - x;
  float inv = *(float*)&x;

  // newton-raphson iteration for accuracy
#ifdef _HIGHER_ACCURACY
  inv *=  2 - inv * f;
#endif

  return inv;
}

/////////////////////////////
// Sine implementation
/////////////////////////////
/**
   https://www.wolframalpha.com/input?i=x%2F%281%2Bx%5E2%2F%286-x%5E2%2B6x%5E2%2F%2820-20x%5E2%2F42%29%29%29+

   five multiplies, three adds, and one divide
   possible extra adds to constrain input value

*/
float fastSin(float x) {

  // constrain input value
  while (x < 0) x += TWO_PI;
  while (x > TWO_PI) x -= TWO_PI;

  boolean neg = false;
  if (x > PI) {
    x -= PI;
    neg = true;
  }

  // do the estimate
  float x2 = x * x;
  float num = 5 * x2 - 177;

  num *= x2;
  num += 1260;
  num *= x;

  float den = 33 * x2 + 1260;
  float value = num / den;

  // shift when required
  return neg ? -value : value;
}

//////////////////////////////////
// Float hacks implementations
//////////////////////////////////
float fastAbs(float f) {
  uint32_t i = *(uint32_t*)&f;
  // unset sign bit
  i &= 0x7FFFFFFF;
  return *(float*)&i;
}

bool fastNonZero(const float x) {
  uint32_t i = *(uint32_t*)&x;
  return (i & 0x7FFFFFFF) != 0;
}

bool fastIsNegative(const float x) {
  uint32_t i = *(uint32_t*)&x;
  return (i & 0x80000000) != 0;
}

/////////////////////////////
// Pow implementation
/////////////////////////////
float fastPow(float a, float b) {
  int iB = (int) b;
  float delta = b - iB;
  float est = _floatToIntPower(a, iB);

  // only need accuracy on affine powers!
  if (fastNonZero(delta)) est *= _fastPow(a, delta);

  return est;
}

float _floatToIntPower( float base, int power) {
  float result = 1.0f;
  while (power != 0) {
    if ((power & 1) == 0) {
      // even
      power >>= 1;
      base *= base;
    } else {
      --power;
      result *= base;
    }
  }
  return result;
}

float _fastPow(const float a, const float b) {
  uint32_t i = *(uint32_t*)&a;
  i = (uint32_t)(b * (float)(i - 1064866805) + 1064866805);
  float r = *(float*)&i;
  return r;
}

/////////////////////////////
// Exp implementation
/////////////////////////////
/**
   Borrowed from https://stackoverflow.com/questions/10552280/fast-exp-calculation-possible-to-improve-accuracy-without-losing-too-much-perfo
*/
float fastExp(const float x) {
  /* exp(x) = 2^i * 2^f; i = floor (log2(e) * x), 0 <= f <= 1 */
  float t = x * 1.442695041f;
  float fi = (float) ((int) t);
  float f = t - fi;
  int i = (int) fi;
  float cvtF = (0.3371894346f * f + 0.657636276f) * f + 1.00172476f; /* compute 2^f */
  uint32_t cvtI = *(uint32_t*)&cvtF;
  cvtI += (i << 23);                                          /* scale by 2^i */
  return *(float*)&cvtI;
}

/////////////////////////////
// Natural log implementation
/////////////////////////////
/**
   Borrowed from https://stackoverflow.com/questions/39821367/very-fast-approximate-logarithm-natural-log-function-in-c
*/
float fastLog(const float a) {
  float m, r, s, t, i, f;
  uint32_t aI = *(uint32_t*)&a;
  uint32_t e = (aI - 0x3f2aaaab) & 0xff800000;
  uint32_t aIMinusE = (aI - e);
  m = *(float*)&aIMinusE;
  i = (float) e * 1.19209290e-7f; // 0x1.0p-23
  /* m in [2/3, 4/3] */
  f = m - 1.0f;
  s = f * f;
  /* Compute log1p(f) for f in [-1/3, 1/3] */
  r = 0.230836749f * f - 0.279208571f; // 0x1.d8c0f0p-3, -0x1.1de8dap-2
  t = 0.331826031f * f - 0.498910338f; // 0x1.53ca34p-2, -0x1.fee25ap-2
  r = r * s + t;
  r = r * s + f;
  r = i * 0.693147182f + r; // 0x1.62e430p-1 // log(2)
  return r;
}
