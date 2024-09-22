#pragma once

// Forward declare Rack to avoid circular dependency
class Rack;

#include "../../constants.h"
#include "../../core/audio/AudioMath.h"
#include "../../core/params.h"
#include "../SynthInterface.h"
#include <array>
#include <cstdlib> // for rand()
#include <iostream>
#include <string>

namespace Synth::Dummy {

class DummyModel : public SynthInterface {
  public:
    // Constructor
    explicit DummyModel(Rack &rack);

    // Method to reset parameters
    void reset() override;

    void initializeParameters();

    // Method to set parameters
    // void setParam(const std::string &name, float val);

    // Method to parse MIDI commands
    void parseMidi(char cmd, char param1, char param2) override;

    // Method to render the next block of audio
    bool renderNextBlock() override;

    // void pushParam(const std::string &name, float val);
    // moved to interface

    // void doPushParam(ParamDefinition param, int val);

    void pushStrParam(const std::string &name, float val);

    bool pushMyParam(const std::string &name, float val);

  private:
    Rack &rack;
    std::array<float, TPH_RACK_BUFFER_SIZE> &buffer; // Use std::array to match Rack's audioBuffer type
    float noiseVolume = 0.1;

    // LPF variables
    float lpfPreviousLeft = 0.0f;
    float lpfPreviousRight = 0.0f;
    float lpfAlpha = 0.0f; // Filter coefficient
    int cutoffHz = 2000;
    float gainLeft = 0.7;
    float gainRight = 0.7;

    void initLPF();
    //
    // Private method to set up parameter definitions and lambdas
    std::unordered_map<std::string, ParamDefinition> parameterDefinitions;
    void setupParams() {
        // Use 'this' in lambdas to access private members directly
        parameterDefinitions = {
            {"pan", {0.1f, 0, false, -1, 2, [this](float v) {
                         // Directly accessing private members
                         this->gainLeft = AudioMath::ccos(v * 0.25f);  // Left gain decreases as panVal goes to 1
                         this->gainRight = AudioMath::csin(v * 0.25f); // Right gain increases as panVal goes to 1
                         std::cout << "setting gain to (L,R)" << this->gainLeft << ", " << this->gainRight << std::endl;
                     }}},
            {"cutoff", {0.5f, 0, true, 20, 8, [this](float v) {
                            this->cutoffHz = AudioMath::logScale2(v, 50, 9);
                            this->initLPF();
                        }}}, // More params can be added
        };
    }
};

} // namespace Synth::Dummy
