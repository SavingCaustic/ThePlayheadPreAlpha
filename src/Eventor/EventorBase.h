#pragma once
#include "./EventorInterface.h"
#include "core/Unit/MidiReciever.h"
#include "core/ext/nlohmann/json.hpp"
#include <cstdint>

class EventorBase : public EventorInterface {
  public:
    EventorBase() {}
    virtual ~EventorBase() = default;

    void setPosition(uint8_t position) {
        this->position = position;
    }

    void parseMidiAndForward(uint8_t cmd, uint8_t param1, uint8_t param2, MidiRecieverInterface &reciever) {
        setMidiTarget(&reciever);
        parseMidi(cmd, param1, param2);
    }

    void setMidiTarget(MidiRecieverInterface *midiReciever) {
        // meh, set every call..
        this->midiReciever = midiReciever;
    }

  protected:
    void sendMidi(uint8_t cmd, uint8_t param1, uint8_t param2) {
        midiReciever->parseMidi(cmd, param1, param2);
    }

    void pushStrParam(const std::string &name, float val) {}

    nlohmann::json getParamDefsAsJson() {
        nlohmann::json jsonOutput;
        return jsonOutput;
    }

    uint8_t position;
    MidiRecieverInterface *midiReciever;
};
