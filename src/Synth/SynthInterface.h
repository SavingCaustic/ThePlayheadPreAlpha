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
class SynthInterface : public MidiRecieverInterface {
  public:
    // Constructor now takes a reference to ErrorWriter
    virtual ~SynthInterface() = default;
    virtual void reset() = 0; // Reset the synth to its default state
    // virtual void parseMidi(uint8_t cmd, uint8_t param1, uint8_t param2) = 0; // Handle MIDI input
    virtual bool renderNextBlock() = 0; // Process the next audio block
    virtual void pushStrParam(const char *name, float val) = 0;
    virtual nlohmann::json getParamDefsAsJSON() = 0;
    virtual void bindBuffers(float *audioBufferLeft, float *audioBufferRight, std::size_t TPH_RACK_RENDER_SIZE) = 0;
    // virtual void updateSetting(Constructor::Record::type, record.value, record.ptr, record.isStereo) = 0;
    virtual void updateSetting(const std::string &type, void *object, uint32_t size, bool isStereo, Destructor::Record &recordDelete) = 0;

    // non virtual (Base methods)
    void handleMidiCC(uint8_t ccNumber, float value);
    void setupCCmapping(const std::string &synthName);

    // this too..
    void logErr(int code, const std::string &message);

  protected:
    // belonging to class, not instance
    // belonging to instance, because they may be overridden by patch settings.
    std::unordered_map<uint8_t, std::array<char, 16>> ccMappings; // Fixed-size char arrays
    float *bufferLeft, *bufferRight;                              // Pointer to audio buffer
    std::size_t bufferSize;                                       // Size of the audio buffer
};
