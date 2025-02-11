#include "./ObjectManager.h"
// #include "../Rack/Rack.h"

ObjectManager::ObjectManager(Rack (&racks)[TPH_RACK_COUNT]) : racks(racks) {}

void ObjectManager::process() {
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
                    destroyEventor(rackID, 1);
                    std::cout << "mounting eventor1 now.. " << std::endl;
                    racks[rackID].setEventor(reinterpret_cast<EventorBase *>(record.ptr), 1);
                    break;
                case Utils::Hash::fnv1a_hash("eventor2"):
                    destroyEventor(rackID, 2);
                    std::cout << "mounting eventor2 now.. " << std::endl;
                    racks[rackID].setEventor(reinterpret_cast<EventorBase *>(record.ptr), 2);
                    break;
                case Utils::Hash::fnv1a_hash("synth"):
                    std::cout << "mounting synth now.. " << std::endl;
                    destroySynth(rackID);
                    racks[rackID].setSynth(reinterpret_cast<SynthBase *>(record.ptr));
                    break;
                case Utils::Hash::fnv1a_hash("effect1"):
                    destroyEffect(rackID, 1);
                    std::cout << "mounting effect1 now.. " << std::endl;
                    racks[rackID].setEffect(reinterpret_cast<EffectBase *>(record.ptr), 1);
                    break;
                case Utils::Hash::fnv1a_hash("effect2"):
                    destroyEffect(rackID, 2);
                    std::cout << "mounting effect2 now.. " << std::endl;
                    racks[rackID].setEffect(reinterpret_cast<EffectBase *>(record.ptr), 2);
                    break;
                }
                break;
            case Utils::Hash::fnv1a_hash("synth"): {
                Destructor::Record recordDelete;
                std::cout << "updating synth-setting now.. (" + methodName + ")" << std::endl;
                // hmm.. tricky to call destroy here since we don't know the name of the property. Better to provide destructor queue?
                racks[rackID].synth->updateSetting(methodName, record.ptr, record.size, record.isStereo, recordDelete);
                // if recordDelete has a set pointer, add it to the queue
                if (recordDelete.ptr != nullptr) {
                    destructorQueue->push(recordDelete);
                }
                break;
            }
            }
        }
    }
}

bool ObjectManager::destroySynth(int rackID) {
    if (racks[rackID].synth != nullptr) {
        Destructor::Record record;
        record.ptr = racks[rackID].synth;
        record.deleter = [](void *ptr) { delete static_cast<SynthBase *>(ptr); }; // Create deleter for SynthBase
        // Push the record to the destructor queue
        if (!destructorQueue->push(record)) {
            std::cout << "Destructor queue is full, could not enqueue the synth to be deleted." << std::endl;
        }
        // std::cout << "destroying synth (inside audio-thread)" << std::endl;
        // delete racks[rackID].synth; // Clean up the old synth
        racks[rackID].synth = nullptr;
        racks[rackID].enabled = false; // Disable the rack if no synth
    }
    return true;
}

bool ObjectManager::destroyEffect(int rackID, int effectSlot) {
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
        if (!destructorQueue->push(record)) {
            std::cout << "Destructor queue is full, could not enqueue the effect to be deleted." << std::endl;
        }
        // std::cout << "destroying synth (inside audio-thread)" << std::endl;
        // delete racks[rackID].synth; // Clean up the old synth
        *effectTarget = nullptr;
    }
    return true;
}

bool ObjectManager::destroyEventor(int rackID, int eventorSlot) {
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
        if (!destructorQueue->push(record)) {
            std::cout << "Destructor queue is full, could not enqueue the eventor to be deleted." << std::endl;
        }
        *eventorTarget = nullptr;
    }
    return true;
}
