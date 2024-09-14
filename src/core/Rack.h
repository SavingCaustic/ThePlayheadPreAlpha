#pragma once

#include "../constants.h"
#include "../synths/Dummy/DummyModel.h"
#include "../synths/SynthInterface.h"
#include "PlayerEngine.h"
#include <array>
#include <iostream>
#include <memory>
#include <string>

class PlayerEngine; // Forward declaration (??!!)

class Rack {
  public:
    // Constructor with reference to PlayerEngine
    explicit Rack(PlayerEngine &engine)
        : playerEngine(engine), audioBuffer{} {
        // No need to setup synth factories here
    }

    std::array<float, TPH_RACK_BUFFER_SIZE> &getAudioBuffer() {
        return audioBuffer;
    }

    void clockReset() {}

    void probeNewClock(float pulse) {}

    void probeNewTick(float pulse) {}

    void render(int num) {
        // Rendering logic here
        if (synth) {
            synth->renderNextBlock();
        } else {
            float *ptr = audioBuffer.data();
            for (int i = 0; i < TPH_RACK_BUFFER_SIZE; ++i) {
                *ptr++ = 0.05 * (((float)rand() / RAND_MAX) * 2.0f - 1.0f); // Noise
            }
        }
    }

    void parseMidi(char cmd, char param1 = 0x00, char param2 = 0x00) {
        if (!(this->hEventor1)) {
            this->synth->parseMidi(cmd, param1, param2);
        } else {
            //$this->hEventor1->parseMidi($command, $param1, $param2);
        }
    }

    bool setSynth(const std::string &synthName) {
        std::cout << "we're setting up synth: " << synthName << std::endl;
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
    }

    std::array<float, TPH_RACK_BUFFER_SIZE> audioBuffer;

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
    std::unique_ptr<SynthInterface> synth;
    std::unique_ptr<SynthInterface> hEventor1; // TOFIX update to EventorInterface
};
