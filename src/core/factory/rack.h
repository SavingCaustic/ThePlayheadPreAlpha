#pragma once
#include "Effect/EffectFactory.h"
#include "Synth/SynthFactory.h"
#include "core/constructor/Queue.h"
#include <core/utils/FNV.h>
#include <iostream>
#include <string>

namespace Factory {

class Rack {
  public:
    static void parse(const std::string &strMethod, const std::string &key, const std::string &strValue, int rackID, Constructor::Queue &constructorQueue) {
        // strMethod is rack.mount.  key = synth, effect0, etc.
        uint32_t unitFNV = Utils::Hash::fnv1a(key);
        std::string queueType = "rack." + key;
        switch (unitFNV) {
        case Utils::Hash::fnv1a_hash("eventor1"):
        case Utils::Hash::fnv1a_hash("eventor2"):
            break;
        case Utils::Hash::fnv1a_hash("synth"): {
            SynthBase *synth = nullptr;
            SynthFactory::setupSynth(synth, strValue);
            constructorQueue.push(synth, sizeof(synth), false, queueType.c_str(), rackID);
            synth = nullptr;
            break;
        }
        case Utils::Hash::fnv1a_hash("effect1"):
        case Utils::Hash::fnv1a_hash("effect2"): {
            EffectBase *effect = nullptr;
            EffectFactory::setupEffect(effect, strValue);
            constructorQueue.push(effect, sizeof(effect), false, queueType.c_str(), rackID);
            effect = nullptr;
            break;
        }
        default:
            std::cerr << "Unknown unit type: " << strMethod << std::endl;
            break;
        }
    }
};

} // namespace Factory
