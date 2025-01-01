#pragma once
#include "./rack.h"
#include "./unit.h"
#include "core/constructor/Queue.h"
#include "core/storage/DocMan.h"
#include "core/storage/Project.h"
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

    static void loadProject(const std::string &strValue, Constructor::Queue &constructorQueue,
                            Storage::DataStore &dataStore) {
        std::string jsonDoc;
        jsonDoc = FileDriver::assetFileRead("projects/" + strValue + "/project.json");
        // Storage::Project::from_json(jsonDoc);
        dataStore.loadProject(jsonDoc);
        //
        for (const auto &rack : dataStore.data.racks) {
            std::cout << "Rack Synth Type: " << rack.synth.type << std::endl;
            if (!rack.synth.type.empty()) {
                // create the synth and pass on constructor queue..
                // this should be move from here to the synth-constructor really..
                Rack::synthSetup(rack.synth.type, 0, constructorQueue);
                for (const auto &setting : rack.synth.settings) {
                    // check with the synth-factory if object is to be created...
                    Unit::parse("set", setting.first, setting.second, 0, "synth", constructorQueue);
                }
                for (const auto &params : rack.synth.params) {
                    // well this is feed to param-queue, not constructor queue..
                    // Unit::parse("synth", setting.first, setting.second, 0, constructorQueue);
                    // std::string name_str(name ? name : "");
                    // MessageIn msg{rack, "synth", name_str.c_str(), value};
                    // messageInBuffer.push(msg);
                }
            }
            if (!rack.effect1.type.empty()) {
                Rack::effectSetup(rack.effect1.type, 0, 1, constructorQueue);
                for (const auto &setting : rack.synth.settings) {
                    // check with the synth-factory if object is to be created...
                    Unit::parse("set", setting.first, setting.second, 0, "effect1", constructorQueue);
                }
            }
        }
        // iterate over collection racks and print each rack syth type as example..
    }
};
} // namespace Factory