#include <Audio.h>

#include "AudioEffectTubeSaturation.h"
#include "AudioEffectParametricEq.h"
#include "AudioEffectGraphicEq.h"
#include "AudioEffectOpticalCompressor.h"
#include "AudioEffectDbx160Comp.h"
#include "AudioEffectFetCompressor.h"
#include "AudioEffectExciter.h"
#include "AudioFilterShelfEq.h"
#include "AudioEffectOutputTransformer.h"
#include "FastMath.h"

#define DEBUG

#define SAMPLERATE AUDIO_SAMPLE_RATE

AudioInputI2S       audioInput;
AudioOutputI2S      audioOutput;
AudioControlSGTL5000 audioShield;

//AudioInputAnalog         adc1;
//AudioOutputAnalog        dac1;

AudioEffectOpticalCompressor optComp;
AudioEffectTubeSaturation tubeSat;
AudioEffectParametricEq paraEq;
AudioEffectDbx160Comp dbxComp;
AudioEffectExciter exciter;
AudioEffectFetCompressor fetComp;
AudioFilterShelfEq shelfEq;
AudioEffectOutputTransformer outTrans;

AudioConnection          patchCord1(audioInput, tubeSat);
AudioConnection          patchCord2(tubeSat, shelfEq);
AudioConnection          patchCord3(shelfEq, optComp);
AudioConnection          patchCord4(optComp, paraEq);
AudioConnection          patchCord5(paraEq, dbxComp);
AudioConnection          patchCord6(dbxComp, fetComp);
AudioConnection          patchCord7(fetComp, outTrans);
//AudioConnection          patchCord8(exciter, outTrans);

AudioConnection          outputL(outTrans, 0, audioOutput, 0);
AudioConnection          outputR(outTrans, 0, audioOutput, 1);

void setup() {
  Serial.begin(9600);

  tubeSat.init(SAMPLERATE);
  paraEq.init(SAMPLERATE);
  optComp.init(SAMPLERATE);
  dbxComp.init(SAMPLERATE);
  fetComp.init(SAMPLERATE);
  exciter.init(SAMPLERATE);
  shelfEq.init(SAMPLERATE);

  //  analogReference(INTERNAL);
  AudioMemory(32);

  // Enable the audio shield and set the output volume.
  audioShield.enable();
  //  audioShield.inputSelect(AUDIO_INPUT_MIC);
  audioShield.inputSelect(AUDIO_INPUT_LINEIN);
  audioShield.volume(0.9);

//  audioShield.audioPostProcessorEnable();
//  audioShield.enhanceBassEnable(); // all we need to do for default bass enhancement settings.
//  audioShield.enhanceBass(5, 127);
//  setBassEnhanceLevel(127, false, 80);
}

void loop() {

#ifdef DEBUG

//  __disable_irq();

  //  powTest();

//  testEffectCpu();

//    showPluginData();

//    testMath();

//  __enable_irq();

//  delay(1000);

#endif

  // put your main code here, to run repeatedly

}

/**
   bassLevel is 0 to 127?
*/
void setBassEnhanceLevel(float bassLevel, bool hpfOn, float cutOffFreq) {
  uint8_t cutOff = 0;
  if (cutOffFreq > 80) ++cutOff;
  if (cutOffFreq > 100) ++cutOff;
  if (cutOffFreq > 125) ++cutOff;
  if (cutOffFreq > 150) ++cutOff;
  if (cutOffFreq > 175) ++cutOff;
  if (cutOffFreq > 200) ++cutOff;

  audioShield.enhanceBass(5, bassLevel, hpfOn ? 1 : 0, cutOff);
}



void logTest() {
  for (float f = 0.f; f < 2.0f; f += 0.01) {
    Serial.print("log: ");
    Serial.print(logf(f));
    Serial.print("   fastLog: ");
    Serial.println(fastLog(f));
  }
}

void powTest() {
  for (float f = 0.f; f < 2.0f; f += 0.01) {
    Serial.print("pow: ");
    Serial.print(powf(f, 3.2));
    Serial.print("   fastPow: ");
    Serial.println(fastPow(f, 3.2));
  }
}

void sqrtTest() {
  for (float f = 0.f; f < 2.0f; f += 0.01) {
    Serial.print("sqrt: ");
    Serial.print(sqrtf(f));
    Serial.print("   fastSqrt: ");
    Serial.println(fastSqrt(f));

  }
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

  Serial.print("DBX 160 Compressor CPU: ");
  Serial.println(dbxComp.processorUsageMax());

  Serial.print("Exciter CPU: ");
  Serial.println(exciter.processorUsage());

  Serial.print("Output Transformer CPU: ");
  Serial.println(outTrans.processorUsage());

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

void showPluginData() {
  Serial.print("OptComp gain reduction: ");
  Serial.println(optComp.getGainReduction());
}
