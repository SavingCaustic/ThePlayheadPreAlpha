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
    static void parse(const std::string &strMethod, const std::string &key, const std::string &strValue, int rackID, Constructor::Queue &constructorQueue) {
        // strMethod is: synth (unit removed)
        uint32_t unitFNV = Utils::Hash::fnv1a(strMethod);
        switch (unitFNV) {
        case Utils::Hash::fnv1a_hash("eventor1"):
        case Utils::Hash::fnv1a_hash("eventor2"):
            break;
        case Utils::Hash::fnv1a_hash("synth"):
            std::cout << "ok we want to mess with the synth setting.." << std::endl;
            synthParse(key, strValue, rackID, constructorQueue);
            break;
        case Utils::Hash::fnv1a_hash("effect1"):
        case Utils::Hash::fnv1a_hash("effect2"):
            break;
        default:
            std::cerr << "Unknown unit type request: " << strMethod << std::endl;
            break;
        }
    }

  private:
    static void synthParse(const std::string &key, const std::string &val, int rackID, Constructor::Queue &constructorQueue) {
        // Delegate to the appropriate synth factory
        // Yeah, hard to get around. We need the appropriate factory, that's it.
        Synth::Subreal::Factory::prepareSetting(key, val, rackID, constructorQueue);
    }
};

} // namespace Factory
