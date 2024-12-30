#pragma once
#include "Effect/EffectFactory.h"
#include "Eventor/EventorFactory.h"
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
        case Utils::Hash::fnv1a_hash("eventor2"): {
            EventorBase *eventor = nullptr;
            EventorFactory::setupEventor(eventor, strValue);
            constructorQueue.push(eventor, sizeof(eventor), false, queueType.c_str(), rackID); // add slot here..
            break;
        }
        case Utils::Hash::fnv1a_hash("synth"): {
            synthSetup(strValue, rackID, constructorQueue);
            // SynthBase *synth = nullptr;
            // SynthFactory::setupSynth(synth, strValue);
            // constructorQueue.push(synth, sizeof(synth), false, queueType.c_str(), rackID);
            // synth = nullptr;
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

    static void synthSetup(const std::string &strValue, int rackID, Constructor::Queue &constructorQueue) {
        SynthBase *synth = nullptr;
        SynthFactory::setupSynth(synth, strValue);
        std::string queueType = "rack.synth";
        constructorQueue.push(synth, sizeof(synth), false, queueType.c_str(), rackID);
        synth = nullptr;
    }

    static void effectSetup(const std::string &strValue, int rackID, int slot, Constructor::Queue &constructorQueue) {
        EffectBase *effect = nullptr;
        EffectFactory::setupEffect(effect, strValue);
        std::string queueType = "rack.effect" + slot;
        std::cout << "setting up " << queueType << std::endl;
        constructorQueue.push(effect, sizeof(effect), false, queueType.c_str(), rackID);
        effect = nullptr;
    }
};

} // namespace Factory
