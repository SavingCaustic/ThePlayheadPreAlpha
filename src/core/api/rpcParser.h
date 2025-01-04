#pragma once
#include "core/factory/project.h"
#include "core/factory/rack.h"
#include "core/factory/server.h"
#include "core/factory/unit.h"
#include "core/storage/DataStore.h"
// #include "core/storage/Project.h"
#include <core/utils/FNV.h>
#include <iostream>
#include <string>

// Forward declaration
namespace Constructor {
class Queue;
}

class RPCParser {
    // Reference to Constructor::Queue
    Constructor::Queue &constructorQueue;
    Storage::DataStore &dataStore;

  public:
    // Constructor with an initializer list to set the reference
    explicit RPCParser(Constructor::Queue &queue, Storage::DataStore &store) : constructorQueue(queue), dataStore(store) {}

    /* examples:

    class.method		key			    value		    rack_id         queuetype
    -----------------------------------------------------------------------------
    server.quit         X               X               X               ---
    project.load        name/latest     filename        X               ---
    project.save_as     name            filename        X               ---
    rack.mount			synth			Subreal		    Y               rack.synth
    rack.mount			effect1			Delay		    Y               rack.effect1
    rack.load           synth           a_patch_name    Y

    unit.synth			lut1_overtones	0.5,0.3,0,1     Y               synth.lut1_overtones
    unit.synth			ch1_sample		filename	    Y               synth.ch1_sample
    --
    we need to extend, *could be that unit is passed as querystring so:
    unit.setting_set    lut1_overtnoes 0.5,0.3,0.1      Y+unit
    unit.param_set      vcf_cutoff      0.5
    unit.reset
    unit.load           no-key          patch-name      Y+unit

    */

    void parse(const std::string &strClass, const std::string &strMethod, const std::string &strKey,
               const std::string &strVal, const std::string &rackID, const std::string &unit) {
        // ok. for simplicity.
        uint32_t classFNV = Utils::Hash::fnv1a(strClass);
        uint32_t methodFNV = Utils::Hash::fnv1a(strMethod);
        switch (classFNV) {
        case Utils::Hash::fnv1a_hash("server"):
            std::cout << "parsing verb: " << strMethod << std::endl;
            Factory::Server::parse(methodFNV, "", "");
            break;
        case Utils::Hash::fnv1a_hash("device"):
            break;
        case Utils::Hash::fnv1a_hash("project"):
            // might need the constructor queue for master effects (outside racks)
            Factory::Project::parse(strMethod, strKey, strVal, constructorQueue, dataStore); // synth, set, monolith
            break;
        case Utils::Hash::fnv1a_hash("rack"):
            std::cout << "rack here we go.." << std::endl;
            Factory::Rack::parse(strMethod, strKey, strVal, stoi(rackID), constructorQueue); // synth, set, monolith
            break;
        case Utils::Hash::fnv1a_hash("unit"):
            std::cout << "unit here we go.." << std::endl;
            Factory::Unit::parse(strMethod, strKey, strVal, stoi(rackID), unit, constructorQueue, dataStore); // synth, "lut1_overtones", "blaha"
            break;
        case Utils::Hash::fnv1a_hash("pattern"):
            break;
        }
    }
};
