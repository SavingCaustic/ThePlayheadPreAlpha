#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>
#include <variant>

// skip this variant stuff, just use strings for values. use stoi etc as needed.
using VariantType = std::variant<int, float, std::string>;

class SettingsManager {
  public:
    static void loadJsonToSettings(const std::string &sFilename, bool isUser, std::unordered_map<std::string, VariantType> &settingsMap);
};
