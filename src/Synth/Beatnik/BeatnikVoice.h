#pragma once
#include "constants.h"
#include "core/audio/AudioMath.h"
#include "core/audio/envelope/ADSFR.h"
#include "core/audio/filter/MultiFilter.h"
#include "core/audio/misc/Easer.h"
#include "core/audio/osc/LUT.h"
#include "core/parameters/params.h"
#include <array>
#include <cstdlib>
#include <iostream>
#include <string>
#include <unordered_map>

namespace Synth::Beatnik {

class Model;

class Voice {
  public:
    Voice(Model &model);

    void reset();

    void noteOn(uint8_t midiNote, float velocity);

    void noteOff();

    float getVCAlevel();

    audio::envelope::ADSFRState getVCAstate();

    bool checkVoiceActive();

    bool renderNextVoiceBlock(std::size_t bufferSize);

    uint8_t notePlaying;

  protected:
    audio::envelope::ADSFR vcaAR;
    audio::envelope::ADSFRSlope vcaARslope;
    audio::misc::Easer oscMixEaser;
    audio::misc::Easer vcaEaser;
    float leftAtt;
    float rightAtt;
    int sampleID;
    float currSamplePos = 0;
    float noteVelocity;
    float velocityLast = 0;
    float vcaEaserVal;
    float tracking;

  private:
    Model &modelRef; // Use reference to Model
};

} // namespace Synth::Beatnik