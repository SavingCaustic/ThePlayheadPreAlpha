#pragma once
#include "constants.h"
#include "core/audio/AudioMath.h"
#include "core/audio/envelope/ADSFR.h"
#include "core/audio/envelope/ASR.h"
#include "core/audio/filter/MultiFilter.h"
#include "core/audio/lfo/LFO.h"
#include "core/audio/misc/Easer.h"
#include "core/audio/osc/LUT.h"
#include <array>
#include <cstdlib>
#include <iostream>
#include <string>

namespace Synth::Subreal {

class Model; // forward declare, model.h included in Voice.cpp

class Voice { //: public VoiceInterface {
              // pass reference to model so we can use AR there. The voice has the ARslope.
  public:
    // Constructor initializes modelRef and LUTs for oscillators
    Voice(Model &model); // Constructor declaration

    void reset();

    void setLUTs(const audio::osc::LUT &lut, int no);

    void noteOn(uint8_t midiNote, float velocity);

    void noteOff();

    float getVCAlevel();

    audio::envelope::ADSFRState getVCAstate();

    bool checkVoiceActive();

    bool renderNextVoiceBlock(std::size_t bufferSize);

    uint8_t notePlaying;

  protected:
    audio::osc::LUTosc osc1; // First oscillator (not a pointer)
    audio::osc::LUTosc osc2; // Second oscillator (not a pointer)
    audio::filter::MultiFilter filter;
    audio::envelope::ADSFRSlope vcaARslope;
    audio::envelope::ADSFRSlope vcfARslope; // should prob be called vcfEnvSlope..
    audio::envelope::ASRSlope pegARslope;

    // audio::misc::Easer oscMixEaser;
    audio::misc::Easer vcaEaser; // maybe replace with audioMath-inline easeLin(delta,target)
    audio::misc::Easer vcfEaser; // maybe replace with audioMath-inline easeLin(delta,target)

    float osc_mix_kv = 0;
    float osc1_fmsens_kv = 0;
    float vcf_cutoff_kv = 0;
    float leftAtt;
    float rightAtt;
    float noteVelocity;
    float velocityLast = 0;
    float vcaEaserVal;
    float vcfEaserVal;
    float tracking;
    float lfo1_ramp_avg;

  private:
    Model &modelRef; // Use reference to Model
    float oscMixEaseOut = 0;
    float fmAmpEaseOut = 0;
    float mixAmplitude;
    int debugCnt = 0;
};
} // namespace Synth::Subreal