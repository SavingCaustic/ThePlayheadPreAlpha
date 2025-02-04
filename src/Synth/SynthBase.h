#pragma once
#include "Synth/SynthInterface.h"
#include "core/Unit/MidiReciever.h"
#include "core/audio/AudioMath.h"
#include "core/ext/nlohmann/json.hpp"
#include "core/hallways/AudioHallway.h"
#include "core/logger/LoggerRec.h"
#include "core/parameters/params.h"
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

// maybe this class should be renamed to SynthInstance or something..
class Rack;

class SynthBase : public SynthInterface {
  public:
    virtual ~SynthBase() = default;

    nlohmann::json getParamDefsAsJson();

    // abstract methods
    void invokeLambda(const int name, const ParamDefinition &paramDef);

    int resolveUPenum(const std::string &name);
    std::string resolveUPname(const int paramID);

    // this could be private.. ehh? called from rack
    void bindBuffers(float *audioBufferLeft, float *audioBufferRight, std::size_t TPH_RACK_RENDER_SIZE, Rack *rack);

    void sendLog(int code, const std::string &message);
    void sendAudioLog();

    void pushStrParam(const char *name, float val);
    // void pushStrParam(const std::string &name, float val);
    void initParams();
    void pushAllParams();
    void indexParams(const int upCount);

  public:
    uint8_t paramIdx = 255; // 255 = unvalid
    LoggerRec logTemp;
    Rack *host = nullptr;
    // static std::unordered_map<int, ParamDefinition> paramDefs;
    // static std::unordered_map<std::string, int> paramIndex;
    std::unordered_map<int, ParamDefinition> paramDefs;
    std::unordered_map<std::string, int> paramIndex;
    std::unordered_map<int, float> paramVals;
    std::unordered_map<std::string, std::string> settingVals;
};
