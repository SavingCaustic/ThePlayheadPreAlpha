#pragma once
#include "core/Unit/MidiReciever.h"
#include "core/destructor/Queue.h"
#include "core/ext/nlohmann/json.hpp"
#include "core/factory/constructor/Queue.h"
#include "core/parameters/params.h"
#include <cstdint>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
class EventorInterface : public MidiRecieverInterface {
  public:
    // Constructor now takes a reference to ErrorWriter
    virtual ~EventorInterface() = default;
    virtual void reset() = 0; // Reset the synth to its default state
    virtual void setMidiTarget(MidiRecieverInterface *midiReciever) = 0;
    virtual void setPosition(uint8_t position) = 0;
    virtual void parseMidiAndForward(uint8_t cmd, uint8_t param1, uint8_t param2, MidiRecieverInterface &reciever) = 0;

    virtual void processClock() = 0;

    // virtual void parseMidi(uint8_t cmd, uint8_t param1, uint8_t param2) = 0; // Handle MIDI input
    virtual void pushStrParam(const std::string &name, float val) = 0;
    virtual nlohmann::json getParamDefsAsJSON() = 0;
    virtual void updateSetting(const std::string &type, void *object, uint32_t size, bool isStereo, Destructor::Record &recordDelete) = 0;

    // non virtual (Base methods)
    void handleMidiCC(uint8_t ccNumber, float value);
    void setupCCmapping(const std::string &synthName);

    // this too..
    void logErr(int code, const std::string &message);

  protected:
    // belonging to class, not instance
    // belonging to instance, because they may be overridden by patch settings.
    std::unordered_map<int, std::string> ccMappings; // MIDI CC -> parameter name mappings
};
