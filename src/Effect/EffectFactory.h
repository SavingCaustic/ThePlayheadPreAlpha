#pragma once
#include "Effect/Chorus/ChorusModel.h"
#include "Effect/Chorus2/Chorus2Model.h"
#include "Effect/Delay/DelayModel.h"
#include <Effect/EffectBase.h>
#include <drivers/FileDriver.h>
#include <nlohmann/json.hpp> // Include the JSON library
// #include "Synth/Sketch/SketchModel.h"

using json = nlohmann::json;

enum class EffectType {
    Delay,
    Chorus,
    Chorus2,
    // Add other synth types here
    _unknown
};

class EffectFactory {
  public:
    static EffectType getEffectType(const std::string &synthName) {
        // i really don't know what i need this for..
        if (synthName == "Delay")
            return EffectType::Delay;
        if (synthName == "Chorus")
            return EffectType::Chorus;
        if (synthName == "Chorus2")
            return EffectType::Chorus2;
        // Add other synth type checks here
        return EffectType::_unknown;
    }

    static bool setupEffect(EffectBase *&newEffect, const std::string &effectName) {
        std::cout << "we're setting up effect: " << effectName << std::endl;
        EffectType type = getEffectType(effectName);

        // overwrite any existing synthPointer, ownership of prev pointer is now in rack
        bool loadOK = true;
        switch (type) {
        case EffectType::Delay:
            newEffect = new Effect::Delay::Model(); // audioBuffer.data(), audioBuffer.size());
            break;
        case EffectType::Chorus:
            newEffect = new Effect::Chorus::Model(); // audioBuffer.data(), audioBuffer.size());
            break;
        case EffectType::Chorus2:
            // synth = new Synth::Sketch::Model(audioBuffer.data(), audioBuffer.size());
            break;
        // Add cases for other synth types here
        default:
            std::cerr << "Unknown effect type: " << effectName << std::endl;
            return false; // Indicate failure
            loadOK = false;
        }
        return true;
    }

    // skipping patch load and save for now..
};