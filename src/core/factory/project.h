#pragma once
#include "./rack.h"
#include "./unit.h"
#include "constructor/Queue.h"
#include "core/storage//DataStore.h"
// #include "core/storage/DocMan.h"
// #include "core/storage/Project.h"
#include "drivers/FileDriver.h"
#include <core/utils/FNV.h>
#include <iostream>
#include <string>

namespace Factory {

// factories are static
class Project {
  public:
    static void parse(const std::string &strMethod, const std::string &key, const std::string &strValue,
                      Constructor::Queue &constructorQueue, Storage::DataStore &dataStore) {
        uint32_t methodFNV = Utils::Hash::fnv1a(strMethod);
        switch (methodFNV) {
        case Utils::Hash::fnv1a_hash("load"):
            std::cout << "hello load project! :)" << std::endl;
            loadProject(strValue, constructorQueue, dataStore);
            break;
        case Utils::Hash::fnv1a_hash("save"):
            break;
        case Utils::Hash::fnv1a_hash("save_as"):
            break;
        case Utils::Hash::fnv1a_hash("delete"):
            break;
        default:
            std::cerr << "Unknown method: " << strMethod << std::endl;
            break;
        }
    }

    static void loadProject(const std::string &strValue, Constructor::Queue &constructorQueue, Storage::DataStore &dataStore) {
        // Step 1: Load the project into the DataStore
        dataStore.loadProject(strValue);
        int rackID = 0;

        // Step 2: Iterate over each rack in the project
        for (const auto &rack : dataStore.project.racks) {
            std::cout << "Rack Synth Type: " << rack.synth.type << std::endl;

            // Step 3: Handle Synth Setup
            if (!rack.synth.type.empty()) {
                Rack::synthSetup(rack.synth.type, rackID, constructorQueue, rack.synth.params);

                // Apply settings for the synth
                for (const auto &setting : rack.synth.settings) {
                    Unit::parse("set", setting.first, setting.second, rackID, "synth", constructorQueue, dataStore);
                }
            }

            // Step 4: Handle Effect1 Setup
            if (!rack.effect1.type.empty()) {
                Rack::effectSetup(rack.effect1.type, rackID, "rack.effect1", constructorQueue);

                // Apply settings for the effect
                for (const auto &setting : rack.effect1.settings) {
                    Unit::parse("set", setting.first, setting.second, rackID, "effect1", constructorQueue, dataStore);
                }
            }
            rackID++;
        }
    }
};
} // namespace Factory