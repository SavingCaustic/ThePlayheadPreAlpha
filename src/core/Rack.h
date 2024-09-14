#pragma once

// #include "../synths/Dummy/DummyModel.h"
// #include "../synths/SynthInterface.h"
#include "../constants.h"
#include "PlayerEngine.h"
#include <array>
#include <iostream>
#include <memory>
#include <string>

class PlayerEngine; // Forward declaration

class Rack {
  public:
    // Constructor with reference to PlayerEngine
    explicit Rack(PlayerEngine &engine)
        : playerEngine(engine), audioBuffer{} {
        // No need to setup synth factories here
    }

    std::array<float, TPH_RACK_RENDER_SIZE> &getAudioBuffer() {
        return audioBuffer;
    }

    void clockReset() {}

    void probeNewClock(float pulse) {}

    void probeNewTick(float pulse) {}

    void render(int num) {
        // Rendering logic here
    }

    bool setSynth(const std::string &synthName) {
        return false;
    }

    /*bool setSynth(const std::string &synthName) {
        SynthType type = getSynthType(synthName);
        switch (type) {
        case SynthType::Dummy:
            synth = std::make_unique<DummyModel>(*this);
            return true;
        // Add cases for other synth types here
        default:
            std::cerr << "Unknown synth type: " << synthName << std::endl;
            return false;
        }
    }*/

    static const std::size_t BUFFER_SIZE = TPH_RACK_RENDER_SIZE; // Fixed buffer size
    std::array<float, BUFFER_SIZE> audioBuffer;

  private:
    enum class SynthType {
        Dummy,
        // Add other synth types here
        Unknown
    };

    SynthType getSynthType(const std::string &synthName) {
        if (synthName == "Dummy")
            return SynthType::Dummy;
        // Add other synth type checks here
        return SynthType::Unknown;
    }

    PlayerEngine &playerEngine; // Reference to PlayerEngine
    // std::unique_ptr<SynthInterface> synth;
};
