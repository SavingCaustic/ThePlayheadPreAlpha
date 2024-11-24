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

    // Method to set the ErrorWriter after construction
    void setErrorWriter(ErrorWriter *errorWriter) {
        errorWriter_ = errorWriter;
    }

    void logErr(int code, const std::string &message);

    // Pure virtual methods that need to be implemented by derived classes
    virtual void reset() = 0;                                                // Reset the synth to its default state
    virtual void parseMidi(uint8_t cmd, uint8_t param1, uint8_t param2) = 0; // Handle MIDI input
    virtual bool renderNextBlock() = 0;                                      // Process the next audio block
    // experimenting with moving push to synth..
    // virtual bool pushMyParam(const std::string &name, float val) = 0;
    // virtual void pushStrParam(const std::string &name, float val) = 0;

    // this could be private.. ehh? called from rack
    nlohmann::json getParamDefsAsJson();

    // abstract methods
    int resolveUPenum(const std::string &name);

    void pushStrParam(const std::string &name, float val);
    void initParams();
    void indexParams(const int upCount);

    void handleMidiCC(u_int8_t ccNumber, float value);
    bool pushMyParam(const std::string &name, float val);

    void invokeLambda(const int name, const ParamDefinition &paramDef);
    void setupCCmapping(const std::string &synthName);

    void sendError(std::string error) {
        // we should really reach playerEngine here, but preferably through a static method.
        //  cant't do this: sPlayerEngine.sendError("huh");
        std::cout << "audio error: " << error << std::endl;
    }

    // Optionally, you can add methods to interact with parameters if needed
    // HERE: This is great! Common stuff for multiple instances of the synth: static
    static std::unordered_map<int, ParamDefinition> paramDefs;
    static std::unordered_map<std::string, int> paramIndex;
    std::unordered_map<int, float> paramVals;

  protected:
    // belonging to class, not instance
    // belonging to instance, because they may be overridden by patch settings.
    std::unordered_map<int, std::string> ccMappings; // MIDI CC -> parameter name mappings
  private:
    ErrorWriter *errorWriter_ = nullptr;
};
