#pragma once
#include "core/constructor/Queue.h"
#include <Synth/Subreal/SubrealFactory.h>
#include <core/utils/FNV.h>
#include <iostream>
#include <string>

// we *need* implementation of unit-factories here. They are called for here.

namespace Factory {

class Unit {
  public:
    static void parse(const std::string &strMethod, const std::string &key, const std::string &strValue,
                      int rackID, const std::string &unit, Constructor::Queue &constructorQueue) {
        // strMethod is: synth (unit removed)
        uint32_t unitFNV = Utils::Hash::fnv1a(unit);
        switch (unitFNV) {
        case Utils::Hash::fnv1a_hash("eventor1"):
        case Utils::Hash::fnv1a_hash("eventor2"):
            break;
        case Utils::Hash::fnv1a_hash("synth"):
            std::cout << "ok we want to mess with the synth setting.." << std::endl;
            synthParse(strMethod, key, strValue, rackID, constructorQueue);
            break;
        case Utils::Hash::fnv1a_hash("effect1"):
        case Utils::Hash::fnv1a_hash("effect2"):
            break;
        default:
            std::cerr << "Unknown unit name in request: " << unit << std::endl;
            break;
        }
    }

  private:
    static void synthParse(const std::string &strMethod, const std::string &key, const std::string &val, int rackID, Constructor::Queue &constructorQueue) {
        // Delegate to the appropriate synth factory
        // Yeah, hard to get around. We need the appropriate factory, that's it.
        // but at the same time, maybe not all units (synths) have factories..
        // well the thing here is really just that we need access to the project map, to adress the right synth.
        Synth::Subreal::Factory::prepareSetting(key, val, rackID, constructorQueue);
    }
};

} // namespace Factory
