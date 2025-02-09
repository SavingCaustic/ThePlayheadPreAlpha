#pragma once
#include <core/ext/nlohmann/json.hpp>
#include <core/parameters/params.h>
#include <string>
#include <unordered_map>
#include <vector>

class EffectInterface {
  public:
    virtual ~EffectInterface() = default;

    // Pure virtual methods that need to be implemented by derived classes
    virtual void reset() = 0;                        // Reset the synth to its default state
    virtual bool renderNextBlock(bool isStereo) = 0; // Process the next audio block
    virtual void pushStrParam(const std::string &name, float val) = 0;
    virtual nlohmann::json getParamDefsAsJSON() = 0;
    virtual void bindBuffers(float *audioBufferLeft, float *audioBufferRight, std::size_t bufferSize) = 0;

    virtual void processClock(const uint8_t clock24) = 0;

    // Optionally, you can add methods to interact with parameters if needed
  protected:
    // belonging to class, not instance
    static std::unordered_map<std::string, ParamDefinition> parameterDefinitions;
    // belonging to instance, because they may be overridden by patch settings.
    std::unordered_map<int, std::string> ccMappings; // MIDI CC -> parameter name mappings
};
