#pragma once
#include <core/params.h>
#include <ext/nlohmann/json.hpp>
#include <string>
#include <unordered_map>
#include <vector>

class SynthInterface {
  public:
    virtual ~SynthInterface() = default;

    // Pure virtual methods that need to be implemented by derived classes
    virtual void reset() = 0;                                       // Reset the synth to its default state
    virtual void parseMidi(char cmd, char param1, char param2) = 0; // Handle MIDI input
    virtual bool renderNextBlock() = 0;                             // Process the next audio block
    // experimenting with moving push to synth..
    // virtual bool pushMyParam(const std::string &name, float val) = 0;
    // virtual void pushStrParam(const std::string &name, float val) = 0;

    // this could be private.. ehh? called from rack
    nlohmann::json getParamDefsAsJson();

    // abstract methods
    void pushStrParam(const std::string &name, float val);
    void initializeParameters();
    void handleMidiCC(int ccNumber, float value);
    bool pushMyParam(const std::string &name, float val);

    void setupCCmapping(const std::string &synthName);

    // Optionally, you can add methods to interact with parameters if needed
  protected:
    std::unordered_map<int, std::string> ccMappings; // MIDI CC -> parameter name mappings
    // Private method to set up parameter definitions and lambdas
    std::unordered_map<std::string, ParamDefinition> parameterDefinitions;
};
