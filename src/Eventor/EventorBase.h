#pragma once
#include "./EventorInterface.h"
#include "Unit/MidiReciever.h"
#include "core/ext/nlohmann/json.hpp"
#include <cstdint>

class EventorBase : public EventorInterface {
  public:
    EventorBase() {}
    virtual ~EventorBase() = default;

    void setPosition(uint8_t position) {
        this->position = position;
    }

    void setMidiTarget(MidiRecieverInterface *midiReciever) {
        // this is called on any eventor load-change so we do NOT have to calculate target at sendMidi..
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
