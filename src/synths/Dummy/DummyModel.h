#pragma once

// Forward declare Rack to avoid circular dependency
class Rack;

#include "../../constants.h"
#include "../SynthInterface.h"
#include <array>
#include <cstdlib> // for rand()
#include <string>

class DummyModel : public SynthInterface {
  public:
    // Constructor
    explicit DummyModel(Rack &rack);

    // Method to reset parameters
    void reset() override;

    // Method to set parameters
    void setParam(const std::string &name, float val) override;

    // Method to parse MIDI commands
    void parseMidi(int cmd, int param1, int param2) override;

    // Method to render the next block of audio
    bool renderNextBlock() override;

  private:
    Rack &rack;
    std::array<float, TPH_RACK_RENDER_SIZE> &buffer; // Use std::array to match Rack's audioBuffer type
};
