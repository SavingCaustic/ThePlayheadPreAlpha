#pragma once
#include "./RackEmittor.h"
#include "Effect/Chorus/ChorusModel.h"
#include "Effect/Chorus2/Chorus2Model.h"
#include "Effect/Delay/DelayModel.h"
#include "Synth/Monolith/MonolithModel.h"
// #include "Synth/Sketch/SketchModel.h"
#include "Synth/Subreal/SubrealModel.h"
#include "core/player/ErrorWriter.h"
//
#include "Eventor/EventorBase.h"
#include "Eventor/EventorInterface.h"
#include "Synth/SynthBase.h"
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

    alignas(32) std::array<float, TPH_RACK_BUFFER_SIZE> audioBuffer;
    bool audioIsStereo;
    RackEmittor emittor;

    Rack() : audioIsStereo(false),
             emittor(audioBuffer.data(), TPH_RACK_BUFFER_SIZE) {}

    ~Rack() {
        delete synth;
        delete effect1;
        delete effect2;
    }

    void setPlayerEngine(PlayerEngine &engine) {
        playerEngine = &engine;
    }

    void setErrorWriter(ErrorWriter &errorWriter) {
        errorWriter_ = &errorWriter;
    }

    // Define the enum for unit types
    enum class UnitType {
        Synth,
        Eventor1,
        Eventor2,
        Effect1,
        Effect2,
        Emittor,
        _unknown // Handle cases where the unit type is invalid
    };

    // Getter method returns a reference to the array
    std::array<float, TPH_RACK_BUFFER_SIZE> &getAudioBuffer() {
        return audioBuffer;
    }

    /* -------------------
    | TIMING LOGIC START |
    ------------------- */

    void clockReset() {}

    void probeNewClock(float pulse) {}

    void probeNewTick(float pulse) {}

    /* ----------------------
    | AUDIO RENDERING START |
    ---------------------- */

    void render(int num) {
        // Rendering logic here
        bool isStereo;
        if (synth) {
            isStereo = synth->renderNextBlock();
        } else {
            isStereo = false;
            float *ptr = audioBuffer.data();
            for (int i = 0; i < TPH_RACK_BUFFER_SIZE; ++i) {
                *ptr++ = 0.05 * (((float)rand() / RAND_MAX) * 2.0f - 1.0f); // Noise
            }
        }
        if (effect1) {
            debugCnt++;
            /*if (debugCnt % 1024 == 0) {
                if (isStereo) {
                    std::cout << "pre-stereo!" << std::endl;
                }
            }*/
            // hey we should have a return argument here - telling if its stereo..
            isStereo = effect1->renderNextBlock(isStereo);
            /*if (debugCnt % 1024 == 0) {
                if (isStereo) {
                    std::cout << "post-stereo!" << std::endl;
                }
            }*/
            // only if effect1
            if (effect2) {
                // hey we should have a return argument here - telling if its stereo..
                isStereo = effect2->renderNextBlock(isStereo);
            }
        }
        isStereo = emittor.process(isStereo);
        //  uhm - maybe always stereo when rack is done..
        this->audioIsStereo = isStereo;
    }

    /* -------------------
    | MIDI LOGIC START |
    ------------------- */

    void parseMidi(uint8_t cmd, uint8_t param1 = 0x00, uint8_t param2 = 0x00) {
        // cc are hardcoded. They are handled by each unit.
        char ccIdx[2];
        if ((cmd & 0xf0) == 0xb0) {
            if (param1 >= 84 && param1 <= 89) {
                ccIdx[0] = static_cast<char>('0' + (param1 - 84));
                ccIdx[1] = '\0';
                passParamToUnit(UnitType::Effect1, ccIdx, param2);
                return;
            } else if (param1 >= 90 && param1 <= 95) {
                // huh - what is this? old artifact?
                passParamToUnit(UnitType::Effect2, "time", param2);
                return;
            }
        }
        // default stuff - run through eventors and synth..
        // it's two different functions. One is parseMidi (old).
        // other is forwardMidi. Only available in eventor.
        if (!(this->eventor1)) {
            // no eventors, straight to synth..
            this->synth->parseMidi(cmd, param1, param2);
        } else {
            // ok, eventor1 is on, now check if eventor2 or synth is target of output.
            if (this->eventor2) {
                this->eventor1->parseMidiAndForward(cmd, param1, param2, *this->eventor2);
            } else {
                this->eventor1->parseMidiAndForward(cmd, param1, param2, *this->synth);
            }
            // this->eventor1->parseMidi(cmd, param1, param2);
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
        return UnitType::_unknown;
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
            if (effect1) {
                effect1->pushStrParam(name, fVal);
            }
            break;
        case UnitType::Effect2:
            if (effect2) {
                effect2->pushStrParam(name, fVal);
            }
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
    EventorInterface *eventor1 = nullptr;
    EventorInterface *eventor2 = nullptr;
    SynthInterface *synth = nullptr;
    EffectInterface *effect1 = nullptr;
    EffectInterface *effect2 = nullptr;
    int debugCnt = 0;

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

    bool setSynth(SynthBase *newSynth) {
        if (synth) {
            std::cout << "destroying synth (inside audio-thread)" << std::endl;
            delete synth; // Clean up the old synth
            synth = nullptr;
        }

        synth = newSynth;
        if (synth) {
            std::cout << "Binding buffers for synth in rack" << std::endl;
            synth->bindBuffers(audioBuffer.data(), audioBuffer.size()); // Bind the buffer here
            this->enabled = true;                                       // Mark the rack as enabled
        } else {
            this->enabled = false; // Disable the rack if no synth
        }

        return synth != nullptr;
    }

    bool setEffect(EffectBase *newEffect, int effectSlot = 1) {
        EffectInterface **effectTarget = nullptr;
        if (effectSlot == 1) {
            effectTarget = &effect1;
        } else {
            effectTarget = &effect2;
        }
        // should already be deleted by ObjectManager
        /*if (*effectTarget) {
            delete *effectTarget;
            *effectTarget = nullptr; // Avoid dangling pointer
        }*/
        *effectTarget = newEffect;
        (*effectTarget)->bindBuffers(audioBuffer.data(), audioBuffer.size());

        return *effectTarget != nullptr;
    }

    bool setEventor(EventorBase *newEventor, int eventorSlot = 1) {
        EventorInterface **eventorTarget = nullptr;
        if (eventorSlot == 1) {
            eventorTarget = &eventor1;
        } else {
            eventorTarget = &eventor2;
        }
        newEventor->setMidiTarget(synth);
        newEventor->setPosition(1);

        *eventorTarget = newEventor;

        return *eventorTarget != nullptr;
    }

    /* stuff below should probably go.. */

    bool setSynthFromStr(const std::string &synthName) {
        // meh - i guess this sohuld go..
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
            synth->bindBuffers(audioBuffer.data(), audioBuffer.size());
            break;
        case SynthType::Subreal:
            synth = new Synth::Subreal::Model();
            synth->bindBuffers(audioBuffer.data(), audioBuffer.size());
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

    bool setEffectFromStr(const std::string &effectName, int effectSlot = 1) {
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
            *effectTarget = new Effect::Delay::Model();
            (*effectTarget)->bindBuffers(audioBuffer.data(), audioBuffer.size());
            break;
        // Add cases for other synth types here
        case EffectType::Chorus:
            *effectTarget = new Effect::Chorus::Model();
            (*effectTarget)->bindBuffers(audioBuffer.data(), audioBuffer.size());
            break;
        case EffectType::Chorus2:
            *effectTarget = new Effect::Chorus2::Model();
            (*effectTarget)->bindBuffers(audioBuffer.data(), audioBuffer.size());
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
};
