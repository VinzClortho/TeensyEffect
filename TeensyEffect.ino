#include <Audio.h>

#include "AudioEffectTubeSaturation.h"
#include "AudioEffectParametricEq.h"
#include "AudioEffectGraphicEq.h"
#include "AudioEffectOpticalCompressor.h"
#include "AudioEffectFetCompressor.h"
#include "AudioEffectExciter.h"
#include "FastMath.h"

#define DEBUG

AudioInputAnalog         adc1;
AudioOutputAnalog        dac1;
AudioEffectParametricEq paraEq;
AudioEffectExciter exciter;
AudioEffectFetCompressor fetComp;
AudioEffectOpticalCompressor optComp;
AudioConnection          patchCord1(adc1, exciter);
AudioConnection          patchCord2(exciter, dac1);
//AudioConnection          patchCord3(optComp, fetComp);
//AudioConnection          patchCord4(fetComp, exciter);
//AudioConnection          patchCord5(exciter, dac1);

void setup() {
  Serial.begin(115200);

  paraEq.init(AUDIO_SAMPLE_RATE_EXACT);
  optComp.init(AUDIO_SAMPLE_RATE_EXACT);
  fetComp.init(AUDIO_SAMPLE_RATE_EXACT);
  exciter.init(AUDIO_SAMPLE_RATE_EXACT);

  analogReference(INTERNAL);
  AudioMemory(64);
}

void loop() {

#ifdef DEBUG

  //  Serial.print("ParametricEq CPU: ");
  //  Serial.println(paraEq.processorUsage());
  //
  //  Serial.print("Optical Compressor CPU: ");
  //  Serial.println(optComp.processorUsageMax());
  //
  //  Serial.print("Fet Compressor CPU: ");
  //  Serial.println(fetComp.processorUsageMax());
  //
  //  Serial.print("Exciter CPU: ");
  //  Serial.println(exciter.processorUsageMax());
  //
  //  Serial.println();

  float r;
  float recip, fRecip;
  float root, fRoot;
  long loops = 10000;

  unsigned long start = millis();

  for (long i = 0; i < loops; ++i) {
    r = random(9) + 1.0f;
    recip = 1.0f / r;
  }
  unsigned long end = millis();
  Serial.print("Actual recip: ");
  Serial.print(recip);
  Serial.print("   microsec: ");
  Serial.println(end - start);


  start = millis();
  for (long i = 0; i < loops; ++i) {
    r = random(9) + 1.0f;
    fRecip = fastRecip(r);
  }
  end = millis();
  Serial.print("  Fast recip: ");
  Serial.print(fRecip);
  Serial.print("   microsec: ");
  Serial.println(end - start);

  start = millis();
  for (long i = 0; i < loops; ++i) {
    r = random(9) + 1.0f;
    root = sqrt(r);
  }
  end = millis();
  Serial.print("        sqrt: ");
  Serial.print(root);
  Serial.print("   microsec: ");
  Serial.println(end - start);

  start = millis();
  for (long i = 0; i < loops; ++i) {
    r = random(9) + 1.0f;
    fRoot = fastSqrt(r);
  }
  end = millis();
  Serial.print("   fast sqrt: ");
  Serial.print(fRoot);
  Serial.print("   microsec: ");
  Serial.println(end - start);

  Serial.println();

  delay(1000);

#endif

  // put your main code here, to run repeatedly:


}
