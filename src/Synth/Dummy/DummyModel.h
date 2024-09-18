#pragma once

// Forward declare Rack to avoid circular dependency
class Rack;

#include "../../constants.h"
#include "../../core/ParamInterfaceBase.h"
#include "../SynthInterface.h"
#include "parameters.h"
#include <array>
#include <cstdlib> // for rand()
#include <string>

class DummyModel : public SynthInterface, public ParamInterfaceBase<src::Synth::Dummy::ParamID> {
  public:
    // Constructor
    explicit DummyModel(Rack &rack);

    // Method to reset parameters
    void reset() override;

    // Method to set parameters
    // void setParam(const std::string &name, float val);

    // Method to parse MIDI commands
    void parseMidi(char cmd, char param1, char param2) override;

    // Method to render the next block of audio
    bool renderNextBlock() override;

    void pushParam(src::Synth::Dummy::ParamID param, float val);

  private:
    Rack &rack;
    std::array<float, TPH_RACK_BUFFER_SIZE> &buffer; // Use std::array to match Rack's audioBuffer type
    float noiseVolume = 0.1;

    // LPF variables
    float lpfPreviousLeft = 0.0f;
    float lpfPreviousRight = 0.0f;
    float lpfAlpha = 0.0f; // Filter coefficient
    int cutoffHz = 2000;
    float pan = 0.5;

    void initLPF();
};
