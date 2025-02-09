#pragma once
#include "./rack.h"
#include "./unit.h"
#include "constructor/Queue.h"
#include "core/hallways/FactoryHallway.h"
#include "core/storage/DataStore.h"
#include "drivers/FileDriver.h"
#include <core/utils/FNV.h>
#include <iostream>
#include <string>

namespace Factory {

// factories are static
class Project {
  public:
    static void parse(const std::string &strMethod, const std::string &key, const std::string &strValue) {
        uint32_t methodFNV = Utils::Hash::fnv1a(strMethod);
        switch (methodFNV) {
        case Utils::Hash::fnv1a_hash("load"):
            std::cout << "hello load project! :)" << std::endl;
            loadProject(strValue);
            break;
        case Utils::Hash::fnv1a_hash("save"):
            break;
        case Utils::Hash::fnv1a_hash("save_as"):
            break;
        case Utils::Hash::fnv1a_hash("delete"):
            break;
        case Utils::Hash::fnv1a_hash("set"):
            projectSet(key, strValue);
            break;
        default:
            std::cerr << "Unknown method: " << strMethod << std::endl;
            break;
        }
    }

    static void projectSet(const std::string &key, const std::string &strValue) {
        Storage::DataStore &dataStoreRef = factoryHallway.storeGetRef();
        // store setting to dataStore
        dataStoreRef.settingsSet(key, strValue);
        // push it to audio
        factoryHallway.test();
        factoryHallway.pushProjectSetting(key, strValue);
    }

    static void loadProject(const std::string &strValue) {
        // Step 1: Load the project into the DataStore
        Storage::DataStore &dataStoreRef = factoryHallway.storeGetRef();
        dataStoreRef.projectLoad(strValue);
        // iterate over project settings and push one at a time..
        for (const auto &[key, strValue] : dataStoreRef.project.settings) {
            std::cout << "Key: " << key << ", Value: " << strValue << std::endl;
            dataStoreRef.settingsSet(key, strValue);
        }
        //
        int rackID = 0;
        // Step 2: Iterate over each rack in the project
        for (const auto &rack : dataStoreRef.project.racks) {
            std::cout << "Rack Synth Type: " << rack.synth.type << std::endl;

            // Step 3: Handle Synth Setup
            if (!rack.synth.type.empty()) {
                Rack::synthSetup(rack.synth.type, rackID, rack.synth.params);

                // Apply settings for the synth
                for (const auto &setting : rack.synth.settings) {
                    Unit::parse("set", setting.first, setting.second, rackID, "synth");
                }
            }

            // Step 4: Handle Effect1 Setup
            if (!rack.effect1.type.empty()) {
                Rack::effectSetup(rack.effect1.type, rackID, "rack.effect1");

                // Apply settings for the effect
                for (const auto &setting : rack.effect1.settings) {
                    Unit::parse("set", setting.first, setting.second, rackID, "effect1");
                }
            }
            rackID++;
        }
    }
};
} // namespace Factory