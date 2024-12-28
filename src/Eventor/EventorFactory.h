#pragma once
#include "Eventor/Fifth/FifthModel.h"
#include "Eventor/Third/ThirdModel.h"
#include <drivers/FileDriver.h>
#include <nlohmann/json.hpp> // Include the JSON library

using json = nlohmann::json;

enum class EventorType {
    Third,
    Fifth,
    // Add other synth types here
    _unknown
};

class EventorFactory {
  public:
    static EventorType getEventorType(const std::string &eventorName) {
        // i really don't know what i need this for..
        if (eventorName == "Third")
            return EventorType::Third;
        if (eventorName == "Fifth")
            return EventorType::Fifth;
        // Add other synth type checks here
        return EventorType::_unknown;
    }

    static bool setupEventor(EventorBase *&newEventor, const std::string &eventorName) {
        std::cout << "we're setting up eventor: " << eventorName << std::endl;
        EventorType type = getEventorType(eventorName);

        // overwrite any existing synthPointer, ownership of prev pointer is now in rack
        bool loadOK = true;
        switch (type) {
        case EventorType::Third:
            newEventor = new Eventor::Third::Model();
            newEventor->setPosition(1);
            break;
        case EventorType::Fifth:
            newEventor = new Eventor::Fifth::Model();
            // oh this sucks..
            newEventor->setPosition(2);
            break;
        // Add cases for other synth types here
        default:
            std::cerr << "Unknown eventor type: " << eventorName << std::endl;
            return false; // Indicate failure
            loadOK = false;
        }
        return true;
    }

    // skipping patch load and save for now..
};