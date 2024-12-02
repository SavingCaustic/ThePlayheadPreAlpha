#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>

class SettingsManager {
  public:
    static void jsonRead(std::unordered_map<std::string, std::string> &settingsMap, const std::string &sFilename);
    static void loadJsonToSettings(const std::string &sFilename, bool isUser, std::unordered_map<std::string, std::string> &settingsMap);
};
