#pragma once
#include <Effect/EffectInterface.h>
#include <core/ext/nlohmann/json.hpp>
#include <core/parameters/params.h>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

// maybe this class should be renamed to SynthInstance or something..

class EffectBase : public EffectInterface {
  public:
    virtual ~EffectBase() = default;

    nlohmann::json getParamDefsAsJson();

    // abstract methods
    void invokeLambda(const int name, const ParamDefinition &paramDef);

    int resolveUPenum(const std::string &name);
    std::string resolveUPname(const int paramID);

    // this could be private.. ehh? called from rack
    void bindBuffers(float *audioBufferLeft, float *audioBufferRight, std::size_t bufferSize);

    void pushStrParam(const std::string &name, float val);
    void initParams();
    void pushAllParams();
    void indexParams(const int upCount);

    // void setupCCmapping(const std::string &effectName);
    //  void handleMidiCC(int ccNumber, float value);

  public:
    // static std::unordered_map<int, ParamDefinition> paramDefs;
    // static std::unordered_map<std::string, int> paramIndex;
    std::unordered_map<int, ParamDefinition> paramDefs;
    std::unordered_map<std::string, int> paramIndex;
    std::unordered_map<int, float> paramVals;
    std::unordered_map<std::string, std::string> settingVals;

  protected:
    float *bufferLeft, *bufferRight; // Pointer to audio buffer
    std::size_t bufferSize;          // Size of the audio buffer
};
