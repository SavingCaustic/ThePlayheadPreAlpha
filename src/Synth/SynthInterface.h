#pragma once
#include <core/ext/nlohmann/json.hpp>
#include <core/parameters/params.h>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

class SynthInterface {
  public:
    virtual ~SynthInterface() = default;

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
    void pushStrParam(const std::string &name, float val);
    void initializeParameters();
    void handleMidiCC(u_int8_t ccNumber, float value);
    bool pushMyParam(const std::string &name, float val);

    void invokeLambda(const std::string &name, const ParamDefinition &paramDef);
    void setupCCmapping(const std::string &synthName);

    void sendError(std::string error) {
        // we should really reach playerEngine here, but preferably through a static method.
        //  cant't do this: sPlayerEngine.sendError("huh");
        std::cout << "audio error: " << error << std::endl;
    }

    // Optionally, you can add methods to interact with parameters if needed
    static std::unordered_map<std::string, ParamDefinition> paramDefs;
    std::unordered_map<std::string, float> paramVals;

  protected:
    // belonging to class, not instance
    // belonging to instance, because they may be overridden by patch settings.
    std::unordered_map<int, std::string> ccMappings; // MIDI CC -> parameter name mappings
};
