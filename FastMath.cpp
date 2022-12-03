#include <Arduino.h>
#include "FastMath.h"

/*
   - borrowed from https://varietyofsound.wordpress.com/2011/02/14/efficient-tanh-computation-using-lamberts-continued-fraction/
   - just 13 multiply/adds plus a division
   - input/output range: -1.0 to 1.0
   - adds odd interval harmonics
*/
float fastTanh(float x) {
  float x2 = x * x;

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

  // over an x delta of 1, so simple math
  float m = y2 - y0;

  for (float x = 0.0; x < 1.0; x += step) {
#ifndef _HAS_FPU
    sum += fastTanh(drive * addEvenOrderHarmonics(m * x + y0));
#else
    sum += fastTanh(drive * addEvenOrderHarmonics(fmaf(m, x, y0)));
#endif
  }

  return sum * step;
}

unsigned char bit_width (int x) {
  return x == 0 ? 1 : 16 - __builtin_clz(x);
}

uint16_t iSqrt(const uint16_t n) {

  unsigned char shift = bit_width(n);
  shift += shift & 1; // round up to next multiple of 2

  int result = 0;

  do {
    shift -= 2;
    result <<= 1; // leftshift the result to make the next guess
    result |= 1;  // guess that the next bit is 1
    result ^= result * result > (n >> shift); // revert if guess too high
  } while (shift != 0);

  return result;

  //  if (n < 2) return n;
  //
  //  unsigned char shift = 2;
  //  while ((n >> shift) != 0) shift += 2;
  //
  //  int result = 0;
  //  while (shift >= 0) {
  //    result = result << 1;
  //    int largeCand = result | 1;
  //    if (largeCand * largeCand <= n >> shift) result = largeCand;
  //    shift -= 2;
  //  }

  return result;
}

#define _USE_FAST_SQRT_RANGE_DIVISORS_TABLE
#ifdef _USE_FAST_SQRT_RANGE_DIVISORS_TABLE
// lookup table to save a multiply, add, subtract, and divide
const float RANGE_DIVISORS[] = {
  1.0, 0.33333334, 0.2, 0.14285715, 0.11111111, 0.09090909, 0.07692308, 0.06666667, 0.05882353, 0.05263158, 0.04761905, 0.04347826, 0.04, 0.037037037, 0.03448276, 0.032258064,
  0.030303031, 0.028571429, 0.027027028, 0.025641026, 0.024390243, 0.023255814, 0.022222223, 0.021276595, 0.020408163, 0.019607844, 0.018867925, 0.018181818, 0.01754386, 0.016949153, 0.016393442, 0.015873017,
  0.015384615, 0.014925373, 0.014492754, 0.014084507, 0.01369863, 0.013333334, 0.012987013, 0.012658228, 0.012345679, 0.012048192, 0.011764706, 0.011494253, 0.011235955, 0.010989011, 0.010752688, 0.010526316,
  0.010309278, 0.01010101, 0.00990099, 0.009708738, 0.00952381, 0.009345794, 0.0091743115, 0.009009009, 0.0088495575, 0.008695652, 0.008547009, 0.008403362, 0.008264462, 0.008130081, 0.008, 0.007874016,
  0.007751938, 0.007633588, 0.007518797, 0.0074074073, 0.00729927, 0.007194245, 0.0070921984, 0.006993007, 0.0068965517, 0.006802721, 0.0067114094, 0.0066225166, 0.006535948, 0.006451613, 0.006369427, 0.006289308,
  0.0062111802, 0.006134969, 0.006060606, 0.005988024, 0.00591716, 0.0058479533, 0.0057803467, 0.0057142857, 0.0056497175, 0.005586592, 0.005524862, 0.0054644807, 0.0054054055, 0.0053475937, 0.005291005, 0.005235602,
  0.005181347, 0.0051282053, 0.005076142, 0.0050251256, 0.0049751243, 0.0049261083, 0.004878049, 0.004830918, 0.004784689, 0.0047393367, 0.0046948357, 0.004651163, 0.004608295, 0.00456621, 0.004524887, 0.004484305,
  0.0044444446, 0.004405286, 0.0043668123, 0.0043290043, 0.0042918455, 0.004255319, 0.004219409, 0.0041841003, 0.004149378, 0.004115226, 0.0040816325, 0.004048583, 0.004016064, 0.003984064, 0.0039525693, 0.003921569,
  0.0038910506, 0.0038610038, 0.0038314175, 0.0038022813, 0.003773585, 0.0037453184, 0.003717472, 0.003690037, 0.0036630037, 0.0036363637, 0.0036101083, 0.0035842294, 0.0035587188, 0.003533569, 0.003508772, 0.0034843206,
  0.0034602077, 0.0034364262, 0.0034129692, 0.0033898305, 0.0033670033, 0.0033444816, 0.003322259, 0.0033003301, 0.0032786885, 0.0032573289, 0.003236246, 0.003215434, 0.0031948881, 0.0031746032, 0.0031545742, 0.0031347962,
  0.0031152647, 0.0030959751, 0.003076923, 0.003058104, 0.0030395137, 0.003021148, 0.003003003, 0.0029850747, 0.002967359, 0.0029498525, 0.0029325513, 0.002915452, 0.0028985508, 0.0028818443, 0.0028653296, 0.0028490028,
  0.0028328612, 0.0028169013, 0.0028011205, 0.0027855153, 0.002770083, 0.002754821, 0.002739726, 0.0027247956, 0.0027100272, 0.0026954177, 0.0026809652, 0.0026666666, 0.0026525198, 0.0026385225, 0.002624672, 0.002610966,
  0.0025974025, 0.0025839794, 0.002570694, 0.0025575447, 0.0025445293, 0.0025316456, 0.0025188916, 0.0025062656, 0.0024937657, 0.0024813896, 0.002469136, 0.0024570024, 0.002444988, 0.00243309, 0.0024213076, 0.0024096386,
  0.0023980816, 0.002386635, 0.002375297, 0.0023640662, 0.002352941, 0.0023419203, 0.0023310024, 0.0023201855, 0.0023094688, 0.0022988506, 0.0022883294, 0.0022779044, 0.0022675737, 0.0022573364, 0.002247191, 0.0022371365,
  0.0022271716, 0.0022172949, 0.0022075055, 0.0021978023, 0.0021881838, 0.0021786492, 0.0021691974, 0.0021598272, 0.0021505377, 0.0021413276, 0.0021321962, 0.0021231424, 0.0021141649, 0.002105263, 0.002096436, 0.0020876827,
  0.002079002, 0.0020703934, 0.0020618557, 0.0020533882, 0.0020449897, 0.00203666, 0.0020283975, 0.002020202, 0.0020120724, 0.002004008, 0.001996008, 0.0019880715, 0.001980198, 0.0019723866, 0.0019646366, 0.0019569471,
};
#endif

// this is about 30% faster than the built in sqrt on a Teensy 3.2!
float fastSqrt(const float x) {

  // this conditional adds a lot of CPU cycles?!
  //   if (x <= 0.0f) return 0.0f;

  float nRoot;
  uint16_t iRoot;

  // get a good starting estimate
  if (x < 1.0f) {
    // scale up to get a decent starting estimate
    iRoot = iSqrt((uint16_t) (x * 65535.0f));
    nRoot = iRoot == 0 ? 0.003906279802663f : (float) iRoot * 0.003906279802663f;
  } else {
    // only find a starting estimate for non-affine values
    iRoot = iSqrt((uint16_t) x);

    // linear interpolate estimate between next perfect square
    // use int math as much as possible for speed
    uint16_t iRoot2 = iRoot * iRoot;
    float delta = x - (float) iRoot2;

#ifdef _USE_FAST_SQRT_RANGE_DIVISORS_TABLE
    // method with lookup table
    nRoot = (float) iRoot + delta * RANGE_DIVISORS[iRoot];
#else
    // method without lookup table
    uint16_t nextRoot = iRoot + 1;
    float range = (float) (nextRoot * nextRoot - iRoot2);
    nRoot = (float) iRoot + delta / range;
#endif
  }

  // only need one iteration of the continued fraction to improve the estimate
  // using the initial estimate for all instances of a and sqrt(x) for better simplification and reduction
#ifndef _HAS_FPU
  return (nRoot * nRoot + x) / (2.0f * nRoot);
#else
  return fmaf(nRoot, nRoot , x) / (2.0f * nRoot);
#endif
}

// This is working with Teensy 3.2 but will hopefully be faster with the 3.5+ FPU
float fastRecip(float x) {

  // TODO: check for FPU and fall back to regular math if not present!

  uint32_t i = *(uint32_t*)&x;
  uint32_t ii = 0x7f3210da - i;
  i = 0x7eb210da - i;
  float y = *(float*)&i;
  float yy = *(float*)&ii;
  const float negXY = -x * y;

#ifndef _HAS_FPU
  y = yy * (1.4143113f + negXY);
  float r = negXY + 1.0f;
  return y * r + y;
#else
  // fmaf is slower without FPU!
  y = yy * fmaf(y, -x, 1.4143113f);
  float r = fmaf(y, -x, 1.0f);
  return fmaf(y, r, y);
#endif
}

/**
   https://stackoverflow.com/questions/12227126/division-as-multiply-and-lut-fast-float-division-reciprocal
*/
float fastRecip2(float f) {
  // get a good estimate via bit twiddling
  uint32_t x = *(uint32_t*)&f;
  x = 0x7EF311C2 - x;
  float inv = *(float*)&x;

  // newton-raphson iteration for accuracy
#ifdef _HIGHER_ACCURACY
#ifndef _HAS_FPU
  inv *=  2 - inv * f;
#else
  // improvement for FPU?
  inv *= fmaf(-inv, f, 2.0f);
#endif
#endif

  return inv;
}

// Quake III:Arena inverse SQRT hack
float Q_rsqrt( float number )
{
  long i;
  float x2, y;
  const float threehalfs = 1.5F;

  x2 = number * 0.5F;
  y  = number;
  i  = * ( long * ) &y;                       // evil floating point bit level hacking
  i  = 0x5f3759df - ( i >> 1 );               // what the fuck?
  y  = * ( float * ) &i;
  y  = y * ( threehalfs - ( x2 * y * y ) );   // 1st iteration

  return y;
}

float fastSqrt2(const float x) {
#ifndef _HAS_FPU
  return 1.0f / Q_rsqrt(x);
#else
  return fastRecip(Q_rsqrt(x));
#endif
}

/**
   https://en.wikipedia.org/wiki/Methods_of_computing_square_roots#Approximations_that_depend_on_the_floating_point_representation

   Fastest so far and averages about 2% error without iteration.
*/
float fastSqrt3(const float x) {
  uint32_t i = *(uint32_t*)&x;
  i -= 1 << 23; /* Subtract 2^m. */
  i >>= 1;    /* Divide by 2. */
  i += 1 << 29; /* Add ((b + 1) / 2) * 2^m. */
  //  i &= 0x7FFFFFFF; /* ensure that sign bit is ot set */
  float f = *(float*)&i;

  // this will improve accuracy but up to triple CPU cycles
#ifdef _HIGHER_ACCURACY
#ifndef _HAS_FPU
  return (f * f + x) / (2.0f * f);
#else
  return fmaf(f, f , x) / (2.0f * f);
#endif
#endif

  return f;   /* Interpret again as float */
}

/**
   https://www.wolframalpha.com/input?i=x%2F%281%2Bx%5E2%2F%286-x%5E2%2B6x%5E2%2F%2820-20x%5E2%2F42%29%29%29+

   five multiplies, three adds, and one divide
   possible extra adds to constrain input value

*/
float fastSin(float x) {

  // constrain input value

#ifndef _HAS_FPU
  while (x < 0) x += TWO_PI;
  while (x > TWO_PI) x -= TWO_PI;
#else
  x = fmod(x, TWO_PI);
#endif

  boolean neg = false;
  if (x > PI) {
    x -= PI;
    neg = true;
  }

  // do the estimate
  float x2 = x * x;
#ifndef _HAS_FPU
  float num = 5 * x2 - 177;
#else
  float num = fmaf(5, x2, -177);
#endif
  num *= x2;
  num += 1260;
  num *= x;
#ifndef _HAS_FPU
  float den = 33 * x2 + 1260;
#else
  float den = fmaf(33 , x2,  1260);
#endif
  float value = num / den;
  // shift when required
  return neg ? -value : value;
}

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
#ifndef _HAS_FPU
  i = (uint32_t)(b * (float)(i - 1064866805) + 1064866805);
#else
  i = (uint32_t)fmaf(b, (float)(i - 1064866805) , 1064866805);
#endif
  float r = *(float*)&i;
  return r;
}

/**
   Borrowed from https://stackoverflow.com/questions/10552280/fast-exp-calculation-possible-to-improve-accuracy-without-losing-too-much-perfo
*/
float fastExp(const float x) {
  /* exp(x) = 2^i * 2^f; i = floor (log2(e) * x), 0 <= f <= 1 */
  float t = x * 1.442695041f;
  float fi = (float) ((int) t);
  float f = t - fi;
  int i = (int) fi;
#ifndef _HAS_FPU
  float cvtF = (0.3371894346f * f + 0.657636276f) * f + 1.00172476f; /* compute 2^f */
#else
  float temp = fmaf(0.3371894346f, f, 0.657636276f);
  float cvtF = fmaf(temp, f, 1.00172476f);
#endif
  uint32_t cvtI = *(uint32_t*)&cvtF;
  cvtI += (i << 23);                                          /* scale by 2^i */
  return *(float*)&cvtI;
}

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
#ifndef _HAS_FPU
  r = 0.230836749f * f - 0.279208571f; // 0x1.d8c0f0p-3, -0x1.1de8dap-2
  t = 0.331826031f * f - 0.498910338f; // 0x1.53ca34p-2, -0x1.fee25ap-2
  r = r * s + t;
  r = r * s + f;
  r = i * 0.693147182f + r; // 0x1.62e430p-1 // log(2)
#else
  r = fmaf(0.230836749f, f, -0.279208571f); // 0x1.d8c0f0p-3, -0x1.1de8dap-2
  t = fmaf(0.331826031f, f, -0.498910338f); // 0x1.53ca34p-2, -0x1.fee25ap-2
  r = fmaf(r, s, t);
  r = fmaf(r, s, f);
  r = fmaf(i, 0.693147182f, r); // 0x1.62e430p-1 // log(2)
#endif
  return r;
}
