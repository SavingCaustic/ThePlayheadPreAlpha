#pragma once
#include "constants.h"
#include "core/constructor/Queue.h"
#include "core/destructor/Queue.h"
#include "core/player/Rack.h"
#include "core/utils/FNV.h"
#include <array>
#include <cmath>
#include <iostream>
#include <string>

class ObjectManager {
  public:
    ObjectManager(Rack (&racks)[TPH_RACK_COUNT]) : racks(racks) {
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
                std::cout << "yey, here" << std::endl;

                size_t delimiterPos = type.find('.'); // uhm.. type format is bad. It's more like target. Where it's going. dots should be used.
                if (delimiterPos != std::string::npos) {
                    className = type.substr(0, delimiterPos);   // Extract part before "."
                    methodName = type.substr(delimiterPos + 1); // Extract part after "."
                } else {
                    // Handle error if no delimiter is found
                    throw std::runtime_error("Invalid type format in record: " + type);
                }
                uint32_t classFNV = Utils::Hash::fnv1a(className);
                uint32_t methodFNV = Utils::Hash::fnv1a(methodName);
                int rackID = record.rackID;
                switch (classFNV) {
                case Utils::Hash::fnv1a_hash("rack"):
                    // maybe it's here we deal with the units?
                    switch (methodFNV) {
                    case Utils::Hash::fnv1a_hash("eventor1"):
                        destroyEventor(0, 1);
                        std::cout << "mounting eventor1 now.. " << std::endl;
                        racks[rackID].setEventor(reinterpret_cast<EventorBase *>(record.ptr), 1);
                        break;
                    case Utils::Hash::fnv1a_hash("eventor2"):
                        destroyEventor(0, 2);
                        std::cout << "mounting eventor2 now.. " << std::endl;
                        racks[rackID].setEventor(reinterpret_cast<EventorBase *>(record.ptr), 2);
                        break;
                    case Utils::Hash::fnv1a_hash("synth"):
                        std::cout << "mounting synth now.. " << std::endl;
                        destroySynth(0);
                        racks[rackID].setSynth(reinterpret_cast<SynthBase *>(record.ptr));
                        break;
                    case Utils::Hash::fnv1a_hash("effect1"):
                        destroyEffect(0, 1);
                        std::cout << "mounting effect1 now.. " << std::endl;
                        racks[rackID].setEffect(reinterpret_cast<EffectBase *>(record.ptr), 1);
                        break;
                    case Utils::Hash::fnv1a_hash("effect2"):
                        destroyEffect(0, 2);
                        std::cout << "mounting effect2 now.. " << std::endl;
                        racks[rackID].setEffect(reinterpret_cast<EffectBase *>(record.ptr), 2);
                        break;
                    }
                    break;
                case Utils::Hash::fnv1a_hash("synth"): {
                    Destructor::Record recordDelete;
                    std::cout << "updating synth-setting now.. " << std::endl;
                    // hmm.. tricky to call destroy here since we don't know the name of the property. Better to provide destructor queue?
                    racks[rackID].synth->updateSetting(methodName, record.ptr, record.size, record.isStereo, recordDelete);
                    // if recordDelete has a set pointer, add it to the queue
                    if (recordDelete.ptr != nullptr) {
                        destructorBuffer->push(recordDelete);
                    }
                    break;
                }
                }
            }
        }
    }

    bool destroySynth(int rackID) {
        if (racks[rackID].synth != nullptr) {
            Destructor::Record record;
            record.ptr = racks[rackID].synth;
            record.deleter = [](void *ptr) { delete static_cast<SynthBase *>(ptr); }; // Create deleter for SynthBase
            // Push the record to the destructor queue
            if (!destructorBuffer->push(record)) {
                std::cout << "Destructor queue is full, could not enqueue the synth to be deleted." << std::endl;
            }
            // std::cout << "destroying synth (inside audio-thread)" << std::endl;
            // delete racks[rackID].synth; // Clean up the old synth
            racks[rackID].synth = nullptr;
            racks[rackID].enabled = false; // Disable the rack if no synth
        }
        return true;
    }

    bool destroyEffect(int rackID, int effectSlot) {
        EffectInterface **effectTarget = nullptr;
        if (effectSlot == 1) {
            effectTarget = &racks[rackID].effect1;
        } else {
            effectTarget = &racks[rackID].effect2;
        }

        if (*effectTarget != nullptr) {
            std::cout << "hey i should not destroy effect yet" << std::endl;
            Destructor::Record record;
            record.ptr = *effectTarget;
            record.deleter = [](void *ptr) { delete static_cast<EffectBase *>(ptr); }; // Create deleter for SynthBase
            // Push the record to the destructor queue
            if (!destructorBuffer->push(record)) {
                std::cout << "Destructor queue is full, could not enqueue the effect to be deleted." << std::endl;
            }
            // std::cout << "destroying synth (inside audio-thread)" << std::endl;
            // delete racks[rackID].synth; // Clean up the old synth
            *effectTarget = nullptr;
        }
        return true;
    }

    bool destroyEventor(int rackID, int eventorSlot) {
        EventorInterface **eventorTarget = nullptr;
        if (eventorSlot == 1) {
            eventorTarget = &racks[rackID].eventor1;
        } else {
            eventorTarget = &racks[rackID].eventor2;
        }

        if (*eventorTarget != nullptr) {
            std::cout << "hey i should not destroy eventor yet" << std::endl;
            Destructor::Record record;
            record.ptr = *eventorTarget;
            record.deleter = [](void *ptr) { delete static_cast<EventorBase *>(ptr); }; // Create deleter for SynthBase
            // Push the record to the destructor queue
            if (!destructorBuffer->push(record)) {
                std::cout << "Destructor queue is full, could not enqueue the eventor to be deleted." << std::endl;
            }
            *eventorTarget = nullptr;
        }
        return true;
    }

    /*void processRack() {
        // say i set up a new synth. I would like to call its model, right?
    }

    void processUnit() {
        // i'm getting the pointer to the sub-resource, will pass it and may run the model..
    }*/

    // all for now..

    Rack (&racks)[TPH_RACK_COUNT]; // Reference to the racks array
    Constructor::Queue *constructorQueue = nullptr;
    Destructor::Queue *destructorBuffer = nullptr;
};
