#pragma once
#include "Effect/EffectFactory.h"
#include "Eventor/EventorFactory.h"
#include "Synth/SynthFactory.h"
#include "constructor/Queue.h"
#include "core/hallways/FactoryHallway.h"
#include <core/utils/FNV.h>
#include <iostream>
#include <string>

namespace Factory {

class Rack {
  public:
    static void parse(const std::string &strMethod, const std::string &key, const std::string &strValue, int rackID) {
        // strMethod is rack.mount.  key = synth, effect0, etc.
        uint32_t unitMethodFNV = Utils::Hash::fnv1a(key + "." + strMethod);
        std::string queueType = "rack." + key;
        switch (unitMethodFNV) {
        case Utils::Hash::fnv1a_hash("eventor1.mount"):
        case Utils::Hash::fnv1a_hash("eventor2.mount"): {
            eventorSetup(strValue, rackID, queueType);
            break;
        }
        case Utils::Hash::fnv1a_hash("synth.mount"): {
            // sorry but we need the patch-name *here*
            synthSetup(strValue, rackID);
            break;
        }
        case Utils::Hash::fnv1a_hash("effect1.mount"):
        case Utils::Hash::fnv1a_hash("effect2.mount"): {
            effectSetup(strValue, rackID, queueType);
            break;
        }
        default:
            std::cerr << "Unknown unit type: " << strMethod << std::endl;
            break;
        }
    }

    static void synthSetup(const std::string &strValue, int rackID, const std::map<std::string, float> &params = {}) {
        SynthBase *synth = nullptr;
        // no synthName = unmount
        if (strValue != "") {
            SynthFactory::setupSynth(synth, strValue, params);
        }
        std::string queueType = "rack.synth";
        bool success = factoryHallway.constructorPush(synth, sizeof(synth), false, queueType.c_str(), rackID);
        if (!success)
            delete synth;
        synth = nullptr;
    }

    static void eventorSetup(const std::string &strValue, int rackID, const std::string &queueType) {
        EventorBase *eventor = nullptr;
        if (strValue != "") {
            EventorFactory::setupEventor(eventor, strValue);
        }
        bool success = factoryHallway.constructorPush(eventor, sizeof(eventor), false, queueType.c_str(), rackID);
        if (!success)
            delete eventor;
        // constructorQueue.push(eventor, sizeof(eventor), false, queueType.c_str(), rackID); // add slot here..
        eventor = nullptr;
    }

    static void effectSetup(const std::string &strValue, int rackID, const std::string &queueType) {
        EffectBase *effect = nullptr;
        if (strValue != "") {
            EffectFactory::setupEffect(effect, strValue);
        }
        bool success = factoryHallway.constructorPush(effect, sizeof(effect), false, queueType.c_str(), rackID);
        if (!success)
            delete effect;
        effect = nullptr;
    }
};

} // namespace Factory
