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
        uint32_t unitMethodFNV = Utils::Hash::fnv1a(key + "." + strMethod);
        std::string queueType = "rack." + key;
        switch (unitMethodFNV) {
        case Utils::Hash::fnv1a_hash("eventor1.mount"):
        case Utils::Hash::fnv1a_hash("eventor2.mount"): {
            eventorSetup(strValue, rackID, queueType, constructorQueue);
            break;
        }
        case Utils::Hash::fnv1a_hash("synth.mount"): {
            synthSetup(strValue, rackID, constructorQueue);
            break;
        }
        case Utils::Hash::fnv1a_hash("effect1.mount"):
        case Utils::Hash::fnv1a_hash("effect2.mount"): {
            effectSetup(strValue, rackID, queueType, constructorQueue);
            break;
        }
        default:
            std::cerr << "Unknown unit type: " << strMethod << std::endl;
            break;
        }
    }

    static void synthSetup(const std::string &strValue, int rackID, Constructor::Queue &constructorQueue) {
        SynthBase *synth = nullptr;
        // no synthName = unmount
        if (strValue != "") {
            SynthFactory::setupSynth(synth, strValue);
        }
        std::string queueType = "rack.synth";
        constructorQueue.push(synth, sizeof(synth), false, queueType.c_str(), rackID);
        synth = nullptr;
    }

    static void eventorSetup(const std::string &strValue, int rackID, const std::string &queueType, Constructor::Queue &constructorQueue) {
        EventorBase *eventor = nullptr;
        if (strValue != "") {
            EventorFactory::setupEventor(eventor, strValue);
        }
        constructorQueue.push(eventor, sizeof(eventor), false, queueType.c_str(), rackID); // add slot here..
        eventor = nullptr;
    }

    static void effectSetup(const std::string &strValue, int rackID, const std::string &queueType, Constructor::Queue &constructorQueue) {
        EffectBase *effect = nullptr;
        if (strValue != "") {
            EffectFactory::setupEffect(effect, strValue);
        }
        constructorQueue.push(effect, sizeof(effect), false, queueType.c_str(), rackID);
        effect = nullptr;
    }
};

} // namespace Factory
