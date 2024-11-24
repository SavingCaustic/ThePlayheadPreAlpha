#include "./SettingsManager.h"
#include "drivers/FileDriver.h"
#include <iostream>

void SettingsManager::loadJsonToSettings(const std::string &sFilename, bool isUser, std::unordered_map<std::string, std::string> &settingsMap) {
    // Read the JSON file (assuming FileDriver::readUserFile returns nlohmann::json)
    std::string fileContent = FileDriver::readUserFile(sFilename); // Assuming this returns a string
    nlohmann::json jsonDoc;
    try {
        jsonDoc = nlohmann::json::parse(fileContent); // Parse the string into JSON
    } catch (const nlohmann::json::parse_error &e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
        return; // Handle error
    }
    for (auto &[jsonKey, jsonValue] : jsonDoc.items()) {
        // Check if the key exists in the settings map
        if (settingsMap.find(jsonKey) != settingsMap.end()) {
            // Determine the type of the current value in the map and update accordingly
            auto &currentValue = settingsMap[jsonKey];
            if (jsonValue.is_string()) {
                currentValue = jsonValue.get<std::string>();
            } else {
                // only strings supported here.."not set";
            }
        }
    }
}
