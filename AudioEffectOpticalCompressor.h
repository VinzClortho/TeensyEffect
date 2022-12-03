#ifndef _AUDIO_STREAM_OPTICAL_COMP_H
#define _AUDIO_STREAM_OPTICAL_COMP_H

#include <Arduino.h>
#include <AudioStream.h>
#include "FastMath.h"

class AudioEffectOpticalCompressor : public AudioStream
{
  public:
    AudioEffectOpticalCompressor() : AudioStream(1, inputQueueArray) {
      // any extra initialization
    }
    void init(float sampleRate);
    virtual void update(void);

    void setThresholdDb(float thresh);
    void setBias(float bias);
    void setMakeupGainDb(float gain);
    void setBlownCapacitor(bool blownCap);
    void setTimeConstant(int tc);
    void setRmsWindowMs(int windowMs);

  private:
    audio_block_t *inputQueueArray[1];

    void setThresholdParams(float thresh, float bias);

    float sampleRate;
    bool blownCap;

    float thresh, threshv, threshvRecip;
    float cthreshv;
    float bias;
    float makeupv;
    float capsc;
    float atcoef, relcoef, rmscoef;

    float spl, maxspl;
    float runave, det, overdb, rundb;
    float cratio, ratio;
    float dcoffset = 0.0;
    float gr, grv;
};

#endif /* _AUDIO_STREAM_OPTICAL_COMP_H */
