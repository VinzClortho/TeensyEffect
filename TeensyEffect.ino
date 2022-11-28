#include <Audio.h>

#include "AudioEffectTubeSaturation.h"
#include "AudioEffectParametricEq.h"
#include "AudioEffectGraphicEq.h"
#include "AudioEffectOpticalCompressor.h"
#include "AudioEffectFetCompressor.h"
#include "AudioEffectExciter.h"

#define DEBUG

AudioInputAnalog         adc1;
AudioOutputAnalog        dac1;
AudioEffectParametricEq paraEq;
AudioEffectExciter exciter;
AudioConnection          patchCord1(adc1, paraEq);
AudioConnection          patchCord2(paraEq, exciter);
AudioConnection          patchCord3(exciter, dac1);

void setup() {
  Serial.begin(115200);

  paraEq.init(AUDIO_SAMPLE_RATE_EXACT);
  exciter.init(AUDIO_SAMPLE_RATE_EXACT);

  analogReference(INTERNAL);
  AudioMemory(4);
}

void loop() {

#ifdef DEBUG

  Serial.print("ParametricEq CPU: ");
  Serial.println(paraEq.processorUsageMax());

  Serial.print("Exciter CPU: ");
  Serial.println(exciter.processorUsageMax());

#endif

  // put your main code here, to run repeatedly:

  delay(1000);
}
