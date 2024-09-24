#pragma once

// Forward declare Rack to avoid circular dependency
class Rack;

#include "Synth/SynthInterface.h"
#include "constants.h"
#include "core/audio/AudioMath.h"
#include "core/params.h"
#include <array>
#include <cstdlib> // for rand()
#include <iostream>
#include <string>
#include <unordered_map>

namespace Synth::Dummy {

enum class FilterType {
    LPF, // Low Pass Filter
    BPF, // Band Pass Filter
    HPF  // High Pass Filter
};

class DummyModel : public SynthInterface {
  public:
    // Constructor
    explicit DummyModel(Rack &rack);
    // Public methods. These should match interface right (contract)
    void reset() override;

    // Method to parse MIDI commands
    void parseMidi(char cmd, char param1, char param2) override;
    bool pushMyParam(const std::string &name, float val);

    // Method to render the next block of audio
    bool renderNextBlock() override;

  private:
    Rack &rack;
    std::array<float, TPH_RACK_BUFFER_SIZE> &buffer; // Use std::array to match Rack's audioBuffer type
    float noiseVolume = 0.4;
    FilterType filterType; // New member variable for filter type

    // LPF variables
    float previousLeft = 0.0f;
    float previousRight = 0.0f;
    float alpha = 0.0f; // Filter coefficient
    int cutoffHz = 2000;
    float gainLeft = 0.7;
    float gainRight = 0.7;

    // Parameter definitiongs
    // std::unordered_map<std::string, ParamDefinition> parameterDefinitions;

    // CC mapping
    // std::unordered_map<int, std::string> ccMappings;

    // Private methods
    // void setupCCmapping(const std::string &path);

    void initLPF();
    void initBPF();
    void initHPF();

    void initializeParameters();
    // Handle incoming MIDI CC messages
    void handleMidiCC(int ccNumber, float value);
    //
    void setupParams() {
        // Use 'this' in lambdas to access private members directly
        parameterDefinitions = {
            {"pan", {0.1f, 0, false, 0, 1, [this](float v) {
                         // Directly accessing private members
                         this->gainLeft = AudioMath::ccos(v * 0.25f);  // Left gain decreases as panVal goes to 1
                         this->gainRight = AudioMath::csin(v * 0.25f); // Right gain increases as panVal goes to 1
                         // create message to messageTransmitter..
                         std::cout << "setting gain to (L,R)" << this->gainLeft << ", " << this->gainRight << std::endl;
                     }}},
            {"cutoff", {0.5f, 0, true, 20, 8, [this](float v) {
                            std::cout << "setting cutoff to " << v << std::endl;
                            this->cutoffHz = v;
                            this->initLPF();
                        }}}, // More params can be added
            {"filter_mode", {0.0f, 3, false, 0, 2, [this](float v) {
                                 // snap somehowm using attributes..
                                 this->filterType = FilterType::LPF;
                                 this->initLPF();
                             }}}, // More params can be added
        };
    }
};

} // namespace Synth::Dummy
