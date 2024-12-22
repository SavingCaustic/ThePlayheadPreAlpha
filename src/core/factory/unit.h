#pragma once
#include <Synth/Subreal/SubrealFactory.h>
#include <core/utils/FNV.h>
#include <iostream>
#include <string>

namespace Factory {

// factories are static

// not sure if factory should be shared across synths and effects etc..
class Unit {
  public:
    static void parse(std::string strMethod, std::string key, std::string strValue, int rackID) {
        // strMethod is synth, effect0 etc.
        uint32_t unitFNV = Utils::Hash::fnv1a(strMethod);
        switch (unitFNV) {
        case Utils::Hash::fnv1a_hash("eventor1"):
        case Utils::Hash::fnv1a_hash("eventor2"):
            break;
        case Utils::Hash::fnv1a_hash("synth"):
            std::cout << "ok we want to mess with the synth setting.." << std::endl;
            synthParse(key, strValue, rackID);
            break;
        case Utils::Hash::fnv1a_hash("effect1"):
        case Utils::Hash::fnv1a_hash("effect2"):
            break;
        }
    }

    static void synthParse(std::string key, std::string val, int rackID) {
        // we need to look at the rack to see what synth it is. How to do that?
        // use the induvidual factory to prepare the object.
        // they are **static**
        // hmm..
        if (true) {
            // since objects may differ, it's the delegated parser that adds to queue..
            Synth::Subreal::Factory::prepareSetting(key, val, rackID);
        }
    }
};

} // namespace Factory