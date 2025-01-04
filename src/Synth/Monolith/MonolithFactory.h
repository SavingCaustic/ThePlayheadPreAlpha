#pragma once
#include "core/constructor/Queue.h"
#include <core/utils/FNV.h>
#include <iostream>
#include <string>

namespace Synth::Monolith {
class Factory {
  public:
    static void prepareSetting(std::string key, std::string value, int rackID, Constructor::Queue &constructorQueue) {
        // well nothing here really..
        //  ok so this bascially pushes the record onto the injector queue..
        std::cout << key << std::endl;
        uint32_t keyFNV = Utils::Hash::fnv1a(key);
    }

    static void reset(int rackID, Constructor::Queue &constructorQueue) {
        // set some default values. maybe move to json later..
        // prepareSetting("lut1_overtones", "0.5,0.3,0.3,0.2,0.2,0.1", rackID, constructorQueue);
        // prepareSetting("lut2_overtones", "0.5,0.0,0.1", rackID, constructorQueue);
    }
};

} // namespace Synth::Monolith