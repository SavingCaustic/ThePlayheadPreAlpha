#pragma once

#include "../Effect/Delay/DelayModel.h"
#include "../Synth/Dummy/DummyModel.h"
#include "../Synth/DummySin/DummySinModel.h"
//
// #include "../Synth/SynthInterface.h"
#include "../constants.h"
// #include "ParamInterfaceBase.h"
#include <array>
#include <cstddef> // for std::size_t
#include <iostream>
#include <memory>
#include <string>

class PlayerEngine; // Forward declaration (??!!)

class Rack {
  public:
    // Method to set PlayerEngine after construction
    void setPlayerEngine(PlayerEngine &engine) {
        playerEngine = &engine;
    }

    alignas(32) std::array<float, TPH_RACK_BUFFER_SIZE> audioBuffer;

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

    // Getter method returns a reference to the array
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
        if (effect1) {
            // hey we should have a return argument here - telling if its stereo..
            effect1->renderNextBlock();
        }
    }

    void parseMidi(uint8_t cmd, uint8_t param1 = 0x00, uint8_t param2 = 0x00) {
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

    std::string getSynthParams() {
        return synth->getParamDefsAsJson();
    }

    bool setSynth(const std::string &synthName) {
        std::cout << "we're setting up synth: " << synthName << std::endl;
        SynthType type = getSynthType(synthName);
        bool loadOK = true;
        switch (type) {
        case SynthType::Dummy:
            synth = std::make_unique<Synth::Dummy::Model>(audioBuffer.data(), audioBuffer.size());
            break;
        case SynthType::DummySin:
            synth = std::make_unique<Synth::DummySin::Model>(audioBuffer.data(), audioBuffer.size());
            break;
        // Add cases for other synth types here
        default:
            std::cerr << "Unknown synth type: " << synthName << std::endl;
            loadOK = false;
        }
        if (loadOK)
            this->enabled = true;
        return (loadOK);
    }

    bool setEffect(const std::string &effectName, int effectSlot = 1) {
        std::cout << "we're setting up effect: " << effectName << std::endl;
        EffectType type = getEffectType(effectName);
        bool loadOK = true;
        switch (type) {
        case EffectType::Delay:
            effect1 = std::make_unique<Effect::Delay::Model>(audioBuffer.data(), audioBuffer.size());
            break;
        // Add cases for other synth types here
        default:
            std::cerr << "Unknown effect type: " << effectName << std::endl;
            loadOK = false;
        }
        if (loadOK)
            this->enabled = true;
        return (loadOK);
    }

    // props
    bool enabled = false;
    std::unique_ptr<SynthInterface> synth;
    std::unique_ptr<EffectInterface> effect1;

  private:
    enum class SynthType {
        Dummy,
        DummySin,
        // Add other synth types here
        Unknown
    };

    SynthType getSynthType(const std::string &synthName) {
        // i really don't know what i need this for..
        if (synthName == "Dummy")
            return SynthType::Dummy;
        if (synthName == "DummySin")
            return SynthType::DummySin;
        // Add other synth type checks here
        return SynthType::Unknown;
    }

    enum class EffectType {
        Delay,
        // Add other synth types here
        Unknown
    };

    EffectType getEffectType(const std::string &effectName) {
        // i really don't know what i need this for..
        if (effectName == "Delay")
            return EffectType::Delay;
        // Add other synth type checks here
        return EffectType::Unknown;
    }

    PlayerEngine *playerEngine; // Reference to PlayerEngine
    // std::unique_ptr<SynthInterface> synth;
    std::unique_ptr<SynthInterface> hEventor1; // TOFIX update to EventorInterface
};
