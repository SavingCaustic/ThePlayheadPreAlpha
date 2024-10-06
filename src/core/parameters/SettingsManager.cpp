#include "./SettingsManager.h"
#include "drivers/FileDriver.h"
#include <iostream>

void SettingsManager::loadJsonToSettings(const std::string &sFilename, bool isUser, std::unordered_map<std::string, VariantType> &settingsMap) {
    // Read the JSON file (assuming FileDriver::readUserFile returns nlohmann::json)
    nlohmann::json jsonDoc = FileDriver::readUserFile(sFilename);

    for (auto &[jsonKey, jsonValue] : jsonDoc.items()) {
        // Check if the key exists in the settings map
        if (settingsMap.find(jsonKey) != settingsMap.end()) {
            // Determine the type of the current value in the map and update accordingly
            auto &currentValue = settingsMap[jsonKey];

            if (std::holds_alternative<int>(currentValue) && jsonValue.is_number_integer()) {
                currentValue = jsonValue.get<int>();
            } else if (std::holds_alternative<float>(currentValue) && jsonValue.is_number_float()) {
                currentValue = jsonValue.get<float>();
            } else if (std::holds_alternative<std::string>(currentValue) && jsonValue.is_string()) {
                currentValue = jsonValue.get<std::string>();
            } else {
                std::cout << "Skipping invalid type for key: " << jsonKey << std::endl;
            }
        } else {
            std::cout << "Skipping invalid key: " << jsonKey << std::endl;
        }
    }
}
