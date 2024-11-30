#pragma once
#include "Effect/Chorus/ChorusModel.h"
#include "Effect/Chorus2/Chorus2Model.h"
#include "Effect/Delay/DelayModel.h"
#include "Synth/Monolith/MonolithModel.h"
// #include "Synth/Sketch/SketchModel.h"
// #include "Synth/Subreal/SubrealModel.h"
#include "core/player/ErrorWriter.h"
//
#include "Synth/SynthInterface.h"
#include "constants.h"
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

    Rack() {}

    ~Rack() {
        delete synth;
        delete effect1;
        delete effect2;
    }

    void setPlayerEngine(PlayerEngine &engine) {
        playerEngine = &engine;
    }

    void setErrorWriter(ErrorWriter &errorWriter) {
        std::cout << "couple recieved " << std::endl;
        errorWriter_ = &errorWriter;
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
        if (effect2) {
            // hey we should have a return argument here - telling if its stereo..
            effect2->renderNextBlock();
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
        return synth->getParamDefsAsJSON();
    }

    // props
    bool enabled = false;
    // std::unique_ptr<SynthInterface> synth;
    // std::unique_ptr<EffectInterface> effect1;
    // std::unique_ptr<EffectInterface> effect2;
    SynthInterface *synth = nullptr;
    EffectInterface *effect1 = nullptr;
    EffectInterface *effect2 = nullptr;

    // I really don't think these methods should be here in Rack, since an effect
    // could be loaded somewhere else. Possibly also a synth..

    enum class SynthType {
        Monolith,
        Subreal,
        Sketch,
        // Add other synth types here
        Unknown
    };

    enum class EffectType {
        Delay,
        Chorus,
        Chorus2,
        // Add other synth types here
        Unknown
    };

    bool setSynth(const std::string &synthName) {
        std::cout << "we're setting up synth: " << synthName << std::endl;
        SynthType type = getSynthType(synthName);
        bool loadOK = true;
        if (synth) {
            delete synth;
            synth = nullptr; // Avoid dangling pointer
        }
        switch (type) {
        case SynthType::Monolith:
            synth = new Synth::Monolith::Model();
            // synth.bind.. audioBuffer.data(), audioBuffer.size());
            break;
        case SynthType::Subreal:
            // synth = new Synth::Subreal::Model(audioBuffer.data(), audioBuffer.size());
            break;
        case SynthType::Sketch:
            // synth = new Synth::Sketch::Model(audioBuffer.data(), audioBuffer.size());
            break;
        // Add cases for other synth types here
        default:
            std::cerr << "Unknown synth type: " << synthName << std::endl;
            loadOK = false;
        }
        if (loadOK) {
            synth->setErrorWriter(errorWriter_);
            this->enabled = true;
        }
        return (loadOK);
    }

    bool
    setEffect(const std::string &effectName, int effectSlot = 1) {
        std::cout << "we're setting up effect: " << effectName << std::endl;
        EffectType type = getEffectType(effectName);
        EffectInterface **effectTarget = nullptr;
        if (effectSlot == 1) {
            effectTarget = &effect1;
        } else {
            // a bit sloppy handling..
            effectTarget = &effect2;
        }

        // Delete the existing effect in the slot, if any
        // and here, the old effect should be enqued for destruction by studio runner.
        if (*effectTarget) {
            delete *effectTarget;
            *effectTarget = nullptr; // Avoid dangling pointer
        }

        bool loadOK = true;
        switch (type) {
        case EffectType::Delay:
            *effectTarget = new Effect::Delay::Model(audioBuffer.data(), audioBuffer.size());
            break;
        // Add cases for other synth types here
        case EffectType::Chorus:
            *effectTarget = new Effect::Chorus::Model(audioBuffer.data(), audioBuffer.size());
            break;
        case EffectType::Chorus2:
            *effectTarget = new Effect::Chorus2::Model(audioBuffer.data(), audioBuffer.size());
            break;
        // Add cases for other synth types here
        default:
            std::cerr << "Unknown effect type: " << effectName << std::endl;
            loadOK = false;
        }
        if (loadOK) {
            synth->setErrorWriter(errorWriter_);
            this->enabled = true;
        }
        return (loadOK);
    }

    SynthType getSynthType(const std::string &synthName) {
        // i really don't know what i need this for..
        if (synthName == "Monolith")
            return SynthType::Monolith;
        if (synthName == "Subreal")
            return SynthType::Subreal;
        if (synthName == "Sketch")
            return SynthType::Sketch;
        // Add other synth type checks here
        return SynthType::Unknown;
    }

    EffectType getEffectType(const std::string &effectName) {
        // i really don't know what i need this for..
        if (effectName == "Delay")
            return EffectType::Delay;
        if (effectName == "Chorus")
            return EffectType::Chorus;
        if (effectName == "Chorus2")
            return EffectType::Chorus2;
        // Add other synth type checks here
        return EffectType::Unknown;
    }

  private:
    PlayerEngine *playerEngine; // Reference to PlayerEngine
    ErrorWriter *errorWriter_ = nullptr;

    // std::unique_ptr<SynthInterface> synth;
    std::unique_ptr<SynthInterface> hEventor1; // TOFIX update to EventorInterface
};
