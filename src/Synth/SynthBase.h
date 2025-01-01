#pragma once
#include "Synth/SynthInterface.h"
#include "core/Unit/MidiReciever.h"
#include "core/ext/nlohmann/json.hpp"
#include "core/parameters/params.h"
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

// maybe this class should be renamed to SynthInstance or something..

class SynthBase : public SynthInterface {
  public:
    virtual ~SynthBase() = default;

    nlohmann::json getParamDefsAsJson();

    // abstract methods
    void invokeLambda(const int name, const ParamDefinition &paramDef);

    int resolveUPenum(const std::string &name);
    std::string resolveUPname(const int paramID);

    // this could be private.. ehh? called from rack
    void bindBuffers(float *audioBuffer, std::size_t bufferSize);

    void pushStrParam(const std::string &name, float val);
    void initParams();
    void pushAllParams();
    void indexParams(const int upCount);

  public:
    // static std::unordered_map<int, ParamDefinition> paramDefs;
    // static std::unordered_map<std::string, int> paramIndex;
    std::unordered_map<int, ParamDefinition> paramDefs;
    std::unordered_map<std::string, int> paramIndex;
    std::unordered_map<int, float> paramVals;
    std::unordered_map<std::string, std::string> settingVals;
};
