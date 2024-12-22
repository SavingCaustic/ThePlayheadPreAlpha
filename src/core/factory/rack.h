#pragma once
#include "Synth/SynthFactory.h"
#include "core/constructor/Queue.h"
#include <Synth/Subreal/SubrealFactory.h>
#include <core/utils/FNV.h>
#include <iostream>
#include <string>

namespace Factory {

class Rack {
  public:
    static void parse(const std::string &strMethod, const std::string &key, const std::string &strValue, int rackID, Constructor::Queue &constructorQueue) {
        // strMethod is synth, effect0, etc.
        uint32_t unitFNV = Utils::Hash::fnv1a(strMethod);
        switch (unitFNV) {
        case Utils::Hash::fnv1a_hash("eventor1"):
        case Utils::Hash::fnv1a_hash("eventor2"):
            break;
        case Utils::Hash::fnv1a_hash("synth"): {
            // SynthType synthType = SynthFactory::getSynthType(key);
            SynthBase *synth = nullptr;
            SynthFactory::setupSynth(synth, key);

            std::cout << "ok we want to mess with the synth setting.." << std::endl;
            synthParse(key, strValue, rackID, constructorQueue);
            break;
        }
        case Utils::Hash::fnv1a_hash("effect1"):
        case Utils::Hash::fnv1a_hash("effect2"):
            break;
        default:
            std::cerr << "Unknown unit type: " << strMethod << std::endl;
            break;
        }
    }

  private:
    static void synthParse(const std::string &key, const std::string &val, int rackID, Constructor::Queue &constructorQueue) {
        // Delegate to the appropriate synth factory
        Synth::Subreal::Factory::prepareSetting(key, val, rackID, constructorQueue);
    }
};

} // namespace Factory
