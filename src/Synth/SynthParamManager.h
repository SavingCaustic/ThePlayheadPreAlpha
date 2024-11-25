#pragma once
#include <core/ext/nlohmann/json.hpp>
#include <core/parameters/params.h>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

class SynthParamManager {
  public:
    virtual ~SynthParamManager() = default;

    // this could be private.. ehh? called from rack
    nlohmann::json getParamDefsAsJson();

    // abstract methods
    void invokeLambda(const int name, const ParamDefinition &paramDef);

    int resolveUPenum(const std::string &name);

    void pushStrParam(const std::string &name, float val);
    void initParams();
    void indexParams(const int upCount);

  public:
    static std::unordered_map<int, ParamDefinition> paramDefs;
    static std::unordered_map<std::string, int> paramIndex;
    std::unordered_map<int, float> paramVals;
};
