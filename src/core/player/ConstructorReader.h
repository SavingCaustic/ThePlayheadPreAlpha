#pragma once
#include "constants.h"
#include "core/constructor/Queue.h"
#include "core/player/Rack.h"
#include "core/utils/FNV.h"
#include <array>
#include <cmath>
#include <iostream>
#include <string>

// responsible for reading queue and initialising objects

/*
uhm
std::unordered_map<uint32_t, std::function<void(const std::string&, const Constructor::Record&)>> handlerMap;

// Initialize the map with class hashes and handler functions
handlerMap[Utils::Hash::fnv1a("synth")] = [](const std::string& methodName, const Constructor::Record& record) {
    racks[0].synth->updateSetting(methodName, record.ptr, record.size, record.isStereo, *constructorQueue);
};



// Initialize the map with class hashes and handler functions
handlerMap[Utils::Hash::fnv1a("synth")] = [](const std::string& methodName, const Constructor::Record& record) {
    // Use the rack number from the record and update the corresponding synth
    int rackNumber = record.rackNumber;  // assuming rackNumber is part of the record
    racks[rackNumber].synth->updateSetting(methodName, record.ptr, record.size, record.isStereo, *constructorQueue);
};

*/

class ConstructorReader {
  public:
    ConstructorReader(Rack (&racks)[TPH_RACK_COUNT]) : racks(racks) {
        // Now you can interact with racks within ConstructorReader
    }

    void process() {
        // check object-injector queue.
        if (!constructorQueue->isEmpty()) {
            auto recordOpt = constructorQueue->pop();
            if (recordOpt.has_value()) {
                const Constructor::Record &record = recordOpt.value();

                // Extract class and method record.type
                std::string type = record.type;
                std::string className, methodName;
                // Handle error if no delimiter is found
                // throw std::runtime_error("controlled demolition with : " + type); //rack.synth

                size_t delimiterPos = type.find('.');
                if (delimiterPos != std::string::npos) {
                    className = type.substr(0, delimiterPos);   // Extract part before "."
                    methodName = type.substr(delimiterPos + 1); // Extract part after "."
                } else {
                    // Handle error if no delimiter is found
                    throw std::runtime_error("Invalid type format in record: " + type);
                }
                uint32_t classFNV = Utils::Hash::fnv1a(className);
                uint32_t methodFNV = Utils::Hash::fnv1a(methodName);
                switch (classFNV) {
                case Utils::Hash::fnv1a_hash("synth"):
                    racks[0].synth->updateSetting(methodName, record.ptr, record.size, record.isStereo, *constructorQueue);
                    break;
                }
            }
        }
    }

    void processRack() {
        // say i set up a new synth. I would like to call its model, right?
    }

    void processUnit() {
        // i'm getting the pointer to the sub-resource, will pass it and may run the model..
    }

    // all for now..

    Constructor::Queue *constructorQueue = nullptr;
    Rack (&racks)[TPH_RACK_COUNT]; // Reference to the racks array
};
