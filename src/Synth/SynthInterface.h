#pragma once
#include "core/player/ErrorWriter.h" // Include ErrorWriter (or forward declare it) to use its methods
#include <core/ext/nlohmann/json.hpp>
#include <core/parameters/params.h>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

class SynthInterface {
  public:
    // Constructor now takes a reference to ErrorWriter
    virtual ~SynthInterface() = default;
    virtual void reset() = 0;                                                // Reset the synth to its default state
    virtual void parseMidi(uint8_t cmd, uint8_t param1, uint8_t param2) = 0; // Handle MIDI input
    virtual bool renderNextBlock() = 0;                                      // Process the next audio block
    virtual void pushStrParam(const std::string &name, float val) = 0;
    virtual nlohmann::json getParamDefsAsJSON() = 0;

    // non virtual (Base methods)
    void setupCCmapping(const std::string &synthName);

    // i'd rather had this in the abstract but no luck..
    void setErrorWriter(ErrorWriter *errorWriter) {
        errorWriter_ = errorWriter;
    }

    void handleMidiCC(u_int8_t ccNumber, float value);

    // this too..
    void logErr(int code, const std::string &message);

  protected:
    // belonging to class, not instance
    // belonging to instance, because they may be overridden by patch settings.
    std::unordered_map<int, std::string> ccMappings; // MIDI CC -> parameter name mappings

  private:
    ErrorWriter *errorWriter_ = nullptr;
};
