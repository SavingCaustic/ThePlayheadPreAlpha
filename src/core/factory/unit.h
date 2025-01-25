#pragma once
#include "Synth/SynthFactory.h"
#include "core/factory/constructor/Queue.h"
#include "core/storage/DataStore.h"
#include <core/utils/FNV.h>
#include <iostream>
#include <string>

// we *need* implementation of unit-factories here. They are called for here.

namespace Factory {

class Unit {
  public:
    static void parse(const std::string &strMethod, const std::string &key, const std::string &strValue,
                      int rackID, const std::string &unit, Constructor::Queue &constructorQueue, Storage::DataStore &dataStore) {
        // strMethod is: synth (unit removed)
        uint32_t unitFNV = Utils::Hash::fnv1a(unit);
        uint32_t methodFNV = Utils::Hash::fnv1a(strMethod);
        switch (unitFNV) {
        case Utils::Hash::fnv1a_hash("eventor1"):
        case Utils::Hash::fnv1a_hash("eventor2"):
            break;
        case Utils::Hash::fnv1a_hash("synth"):
            switch (methodFNV) {
            case Utils::Hash::fnv1a_hash("patchLoad"): {
                dataStore.loadSynthPatch(strValue, rackID);
                // Reference to the synth for convenience
                auto &synth = dataStore.project.racks[rackID].synth;
                // do same stuff as in project. Code copied for now..
                if (!synth.type.empty()) {
                    Rack::synthSetup(synth.type, rackID, constructorQueue, synth.params);

                    // Apply settings for the synth
                    for (const auto &setting : synth.settings) {
                        Unit::parse("set", setting.first, setting.second, rackID, "synth", constructorQueue, dataStore);
                    }
                }
                break;
            }
            case Utils::Hash::fnv1a_hash("set"):
                synthSet(strMethod, key, strValue, rackID, dataStore, constructorQueue);
                break;
            }
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
    static void synthSet(const std::string &strMethod, const std::string &key, const std::string &val, int rackID, Storage::DataStore &dataStore, Constructor::Queue &constructorQueue) {
        // hmm.. load and save maybe not here.. It's all about settings..
        //  Retrieve the synth type from the dataStore
        const std::string &synthType = dataStore.project.racks[rackID].synth.type;
        if (synthType.empty()) {
            std::cerr << "Error: No synth type found for rack ID " << rackID << std::endl;
            return;
        }

        // Use SynthFactory to get the correct synth type
        SynthType type = SynthFactory::getSynthType(synthType);

        // Route to the correct factory based on the synth type
        switch (type) {
        case SynthType::Subreal:
            // Use Subreal's factory for its specific operations
            Synth::Subreal::Factory::prepareSetting(key, val, rackID, constructorQueue);
            break;
        case SynthType::Monolith:
            // Use Monolith's factory to handle specific operations
            Synth::Monolith::Factory::prepareSetting(key, val, rackID, constructorQueue);
            break;
        case SynthType::Beatnik:
            // Use Monolith's factory to handle specific operations
            Synth::Beatnik::Factory::prepareSetting(key, val, rackID, constructorQueue);
            break;
        case SynthType::Sketch:
            // If you have a Sketch factory, handle it here
            break;
        default:
            std::cerr << "Error: Unknown synth type '" << synthType << "' for rack ID " << rackID << std::endl;
            break;
        }
    }
};

} // namespace Factory
