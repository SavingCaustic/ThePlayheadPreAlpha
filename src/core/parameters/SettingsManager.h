#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>
#include <variant>

using VariantType = std::variant<int, float, std::string>;

class SettingsManager {
  public:
    static void loadJsonToSettings(const std::string &sFilename, bool isUser, std::unordered_map<std::string, VariantType> &settingsMap);
};
