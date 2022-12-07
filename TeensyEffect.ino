#include <Audio.h>

#include "AudioEffectTubeSaturation.h"
#include "AudioEffectParametricEq.h"
#include "AudioEffectGraphicEq.h"
#include "AudioEffectOpticalCompressor.h"
#include "AudioEffectFetCompressor.h"
#include "AudioEffectExciter.h"
#include "AudioEffectOutputTransformer.h"
#include "FastMath.h"

#define DEBUG

AudioInputAnalog         adc1;
AudioEffectTubeSaturation tubeSat;
AudioEffectParametricEq paraEq;
AudioEffectOpticalCompressor optComp;
AudioEffectExciter exciter;
AudioEffectFetCompressor fetComp;
AudioEffectOutputTransformer outTrans;
AudioOutputAnalog        dac1;

AudioConnection          patchCord1(adc1, tubeSat);
AudioConnection          patchCord2(tubeSat, paraEq);
AudioConnection          patchCord3(paraEq, optComp);
AudioConnection          patchCord4(optComp, exciter);
AudioConnection          patchCord5(exciter, outTrans);
AudioConnection          patchCord6(outTrans, dac1);

void setup() {
  Serial.begin(9600);

  tubeSat.init(AUDIO_SAMPLE_RATE_EXACT);
  paraEq.init(AUDIO_SAMPLE_RATE_EXACT);
  optComp.init(AUDIO_SAMPLE_RATE_EXACT);
  fetComp.init(AUDIO_SAMPLE_RATE_EXACT);
  exciter.init(AUDIO_SAMPLE_RATE_EXACT);

  analogReference(INTERNAL);
  AudioMemory(32);

}

void loop() {

#ifdef DEBUG

  __disable_irq();

  testEffectCpu();

  //    testMath();

  __enable_irq();

  delay(1000);

#endif

  // put your main code here, to run repeatedly

}

void testEffectCpu() {
  Serial.print("ParametricEq CPU: ");
  Serial.println(paraEq.processorUsage());

  Serial.print("Tube Saturation CPU: ");
  Serial.println(tubeSat.processorUsageMax());

  Serial.print("Optical Compressor CPU: ");
  Serial.println(optComp.processorUsageMax());

  Serial.print("Fet Compressor CPU: ");
  Serial.println(fetComp.processorUsageMax());

  Serial.print("Exciter CPU: ");
  Serial.println(exciter.processorUsage());

  Serial.println();
}

void testMath() {
  float r;
  float recip, fRecip;
  float root, fRoot;
  long loops = 100000;

  unsigned long start, end;

  start = millis();
  for (long i = 0; i < loops; ++i) {
    r = random(2);
    root = sqrtf(r);
  }
  end = millis();
  Serial.print(" Actual sqrt: ");
  Serial.print(root);
  Serial.print("   ms: ");
  Serial.println(end - start);


  for (long i = 0; i < loops; ++i) {
    r = random(2);
    root = sqrtf(r);
  }
  end = millis();
  Serial.print(" Actual sqrt: ");
  Serial.print(root);
  Serial.print("   ms: ");
  Serial.println(end - start);

  start = millis();
  for (long i = 0; i < loops; ++i) {
    r = random(2);
    fRoot = fastSqrt(r);
  }
  end = millis();
  Serial.print("   fast sqrt: ");
  Serial.print(fRoot);
  Serial.print("   ms: ");
  Serial.println(end - start);

  start = millis();
  for (long i = 0; i < loops; ++i) {
    r = random(2) + 0.01f;
    recip = 1.0f / r;
  }
  end = millis();
  Serial.print("Actual recip: ");
  Serial.print(recip);
  Serial.print("   ms: ");
  Serial.println(end - start);

  start = millis();
  for (long i = 0; i < loops; ++i) {
    r = random(2) + 0.01f;
    fRecip = fastRecip(r);
  }
  end = millis();
  Serial.print("  fast recip: ");
  Serial.print(fRecip);
  Serial.print("   ms: ");
  Serial.println(end - start);

  float p;
  float _pow, fPow;

  start = millis();
  for (long i = 0; i < loops; ++i) {
    r = random(20) + 0.01f;
    p = random(10);
    _pow = powf(r, p);
  }
  end = millis();
  Serial.print("  Actual pow: ");
  Serial.print(_pow);
  Serial.print("   ms: ");
  Serial.println(end - start);

  start = millis();
  for (long i = 0; i < loops; ++i) {
    r = random(20) + 0.01f;
    p = random(10);
    fPow = fastPow(r, p);
  }
  end = millis();
  Serial.print("    fast pow: ");
  Serial.print(fPow);
  Serial.print("   ms: ");
  Serial.println(end - start);

  float realExp, fExp;

  start = millis();
  for (long i = 0; i < loops; ++i) {
    p = random(10);
    realExp = expf(p);
  }
  end = millis();
  Serial.print("  Actual exp: ");
  Serial.print(realExp);
  Serial.print("   ms: ");
  Serial.println(end - start);

  start = millis();
  for (long i = 0; i < loops; ++i) {
    p = random(10);
    fExp = fastExp(p);
  }
  end = millis();
  Serial.print("    fast exp: ");
  Serial.print(fExp);
  Serial.print("   ms: ");
  Serial.println(end - start);


  float realLog, fLog;

  start = millis();
  for (long i = 0; i < loops; ++i) {
    p = random(10);
    realLog = logf(p);
  }
  end = millis();
  Serial.print("  Actual log: ");
  Serial.print(realLog);
  Serial.print("   ms: ");
  Serial.println(end - start);

  start = millis();
  for (long i = 0; i < loops; ++i) {
    p = random(10);
    fLog = fastLog(p);
  }
  end = millis();
  Serial.print("    fast log: ");
  Serial.print(fLog);
  Serial.print("   ms: ");
  Serial.println(end - start);

  float realTanH, fTanH;

  start = millis();
  for (long i = 0; i < loops; ++i) {
    p = random(PI);
    realTanH = tanhf(p);
  }
  end = millis();
  Serial.print(" Actual tanh: ");
  Serial.print(realTanH);
  Serial.print("   ms: ");
  Serial.println(end - start);

  start = millis();
  for (long i = 0; i < loops; ++i) {
    p = random(PI);
    fTanH = fastTanh(p);
  }
  end = millis();
  Serial.print("   fast tanh: ");
  Serial.print(fTanH);
  Serial.print("   ms: ");
  Serial.println(end - start);


  float realSin, fSin;

  start = millis();
  for (long i = 0; i < loops; ++i) {
    p = random(PI);
    realSin = sinf(p);
  }
  end = millis();
  Serial.print("  Actual sin: ");
  Serial.print(realSin);
  Serial.print("   ms: ");
  Serial.println(end - start);

  start = millis();
  for (long i = 0; i < loops; ++i) {
    p = random(PI);
    fSin = fastSin(p);
  }
  end = millis();
  Serial.print("    fast sin: ");
  Serial.print(fSin);
  Serial.print("   ms: ");
  Serial.println(end - start);

  //////////////////////////////////
  // specific value tests
  //////////////////////////////////

  //  r = random(10);
  //  Serial.print("Recip ");
  //  recip = 1.0f / r;
  //  Serial.print(recip);
  //  Serial.print(" -> ");
  //  Serial.println(fastRecip2(r));
  //
  //  Serial.print("Pow 3.2, 3.9 = ");
  //  _pow = powf(3.2, 3.9);
  //  fPow = fastPow(3.2, 3.9);
  //  Serial.print(_pow);
  //  Serial.print(" -> ");
  //  Serial.println(fPow);

  Serial.println();
}
