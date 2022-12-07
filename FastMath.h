#ifndef _FAST_MATH_H
#define _FAST_MATH_H

// #define _HIGHER_ACCURACY

#define NUM_CHANNELS 1
#define AUDIO_BLOCK_SAMPLES 128
inline
#define C_DC_ADD  10E-30
#define C_DENORM 10E-30
#define C_AMP_DB 8.65617025
#define LOG_TO_DB  8.6858896380650365530225783783321 // 20 / ln(10)
#define DB_TO_LOG  0.11512925464970228420089957273422 // ln(10) / 20 
#define BLOWN_CAP_SCALAR 2.08136898
#define FLOAT_TO_INT  32768
#define INT_TO_FLOAT  1.0f/FLOAT_TO_INT

#define AA_STEP_1 1.0
#define AA_STEP_2 1.0/2.0
#define AA_STEP_3 1.0/3.0
#define AA_STEP_4 1.0/4.0
#define AA_STEP_5 1.0/5.0
#define AA_STEP_6 1.0/6.0
#define AA_STEP_7 1.0/7.0
#define AA_STEP_8 1.0/8.0

float addEvenOrderHarmonics(float x);
float saturation(float y0, float y2, int antiAliasSteps, float drive);

float fastAbs(float f);
float fastExp(const float x);
bool fastIsNegative(const float x);
float fastLog(const float a);
bool fastNonZero(const float x);
float fastPow(float a, float b);
float fastRecip(const float f);
float fastSin(float x);
float fastSqrt(const float x);
float fastTanh(float x);

// implementation methods not intended for general use
float _fastPow(const float a, const float b);
float _floatToIntPower(float base, int power);

#endif /* _FAST_MATH_H */
