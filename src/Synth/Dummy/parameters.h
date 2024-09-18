#pragma once
namespace src::Synth::Dummy {

#include <unordered_map>
#include <string>
#include <vector>

enum class ParamID {
    Pan,
    Filter_freq,
    Filter_type,
    Unknown
};

enum class ParamType {
    Dec,
    Opt,
    Unknown
};

enum class Filter_typeOptions {
    LPF,
    HPF,
    BPF,
    Unknown
};

const std::unordered_map<std::string, ParamID> paramMap = {
    {"pan", ParamID::Pan},
    {"filter_freq", ParamID::Filter_freq},
    {"filter_type", ParamID::Filter_type},
};

const std::unordered_map<ParamID, std::pair<ParamType, double>> defaultValues = {
    {ParamID::Pan, {ParamType::Dec, 0.5}},
    {ParamID::Filter_freq, {ParamType::Dec, 0.5}},
    {ParamID::Filter_type, {ParamType::Opt, 1}},
};

const std::unordered_map<int, Filter_typeOptions> Filter_typeOptionsMap = {
    {0, Filter_typeOptions::LPF},
    {1, Filter_typeOptions::HPF},
    {2, Filter_typeOptions::BPF},
};

} // namespace src::Synth::Dummy
