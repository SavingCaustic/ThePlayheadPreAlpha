#pragma once

#include "../Synth/Dummy/DummyModel.h"
//
#include "../Synth/SynthInterface.h"
#include "../constants.h"
// #include "ParamInterfaceBase.h"
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

    // Define the enum for unit types
    enum class UnitType {
        Synth,
        Emittor,
        Eventor1,
        Eventor2,
        Effect1,
        Effect2,
        Unknown // Handle cases where the unit type is invalid
    };

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

    // Convert string to enum
    static UnitType stringToUnitType(const char *unit) {
        if (std::strcmp(unit, "synth") == 0) {
            return UnitType::Synth;
        } else if (std::strcmp(unit, "emittor") == 0) {
            return UnitType::Emittor;
        } else if (std::strcmp(unit, "eventor1") == 0) {
            return UnitType::Eventor1;
        } else if (std::strcmp(unit, "eventor2") == 0) {
            return UnitType::Eventor2;
        } else if (std::strcmp(unit, "effect1") == 0) {
            return UnitType::Effect1;
        } else if (std::strcmp(unit, "effect2") == 0) {
            return UnitType::Effect2;
        }
        return UnitType::Unknown;
    }

    // passParamToUnit now takes the enum type instead of a string
    void passParamToUnit(UnitType unit, const char *name, int val) {
        float fVal = static_cast<float>(val) / 127.0f;
        switch (unit) {
        case UnitType::Synth:
            synth->pushStrParam(name, fVal);
            break;
        case UnitType::Emittor:
            // Do something else
            break;
        case UnitType::Eventor1:
        case UnitType::Eventor2:
            // Handle eventors
            break;
        case UnitType::Effect1:
        case UnitType::Effect2:
            // Handle effects
            break;
        default:
            // Handle unknown or unsupported units
            std::cerr << "Unknown unit type!" << std::endl;
            break;
        }
    }

    bool setSynth(const std::string &synthName) {
        std::cout << "we're setting up synth: " << synthName << std::endl;
        SynthType type = getSynthType(synthName);
        switch (type) {
        case SynthType::Dummy:
            synth = std::make_unique<Synth::Dummy::DummyModel>(*this);
            return true;
        // Add cases for other synth types here
        default:
            std::cerr << "Unknown synth type: " << synthName << std::endl;
            return false;
        }
    }

    std::array<float, TPH_RACK_BUFFER_SIZE> audioBuffer;
    // i dont wanna have this public but until passing on works better..
    std::unique_ptr<SynthInterface> synth;

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
    std::unique_ptr<SynthInterface> hEventor1; // TOFIX update to EventorInterface
};
