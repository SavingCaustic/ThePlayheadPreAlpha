#include "./SettingsManager.h"
#include "drivers/FileDriver.h"
#include <iostream>

void SettingsManager::loadJsonToSettings(const std::string &sFilename, bool isUser, std::unordered_map<std::string, std::string> &settingsMap) {
    // Read the JSON file (assuming FileDriver::readUserFile returns nlohmann::json)
    nlohmann::json jsonDoc = FileDriver::readUserFile(sFilename);

    for (auto &[jsonKey, jsonValue] : jsonDoc.items()) {
        // Check if the key exists in the settings map
        if (settingsMap.find(jsonKey) != settingsMap.end()) {
            // Determine the type of the current value in the map and update accordingly
            auto &currentValue = settingsMap[jsonKey];
            if (jsonValue.is_string()) {
                currentValue = jsonValue.get<std::string>();
            } else {
                "not set";
            }
        }
    }
}
