#include "./SettingsManager.h"
#include "drivers/FileDriver.h"
#include <iostream>

void SettingsManager::jsonRead(std::unordered_map<std::string, std::string> &settingsMap, const std::string &sFilename) {
    // Read the file in /assets (read-only)
    std::string assetsContent = FileDriver::readAssetFile(sFilename); // Assuming this returns the file as a string
    nlohmann::json assetsJson;
    try {
        assetsJson = nlohmann::json::parse(assetsContent); // Parse the string into JSON
    } catch (const nlohmann::json::parse_error &e) {
        std::cerr << "Error parsing JSON from assets file: " << e.what() << std::endl;
        return;
    }

    // Populate settingsMap with keys/values from the assets file
    for (const auto &[key, value] : assetsJson.items()) {
        if (value.is_string()) {
            settingsMap[key] = value.get<std::string>();
        } else {
            std::cerr << "Invalid value type in assets file for key: " << key << std::endl;
        }
    }

    // Now read the optional file in /user (read/write)
    std::string userContent = FileDriver::readUserFile(sFilename); // Assuming this also returns the file as a string
    if (!userContent.empty()) {
        nlohmann::json userJson;
        try {
            userJson = nlohmann::json::parse(userContent);
        } catch (const nlohmann::json::parse_error &e) {
            std::cerr << "Error parsing JSON from user file: " << e.what() << std::endl;
            return;
        }

        // Update settingsMap with values from the user file if the keys already exist
        for (const auto &[key, value] : userJson.items()) {
            if (settingsMap.find(key) != settingsMap.end() && value.is_string()) {
                settingsMap[key] = value.get<std::string>();
            } else if (settingsMap.find(key) == settingsMap.end()) {
                std::cerr << "Key in user file not recognized: " << key << std::endl;
            }
        }
    }
}

void SettingsManager::loadJsonToSettings(const std::string &sFilename, bool isUser, std::unordered_map<std::string, std::string> &settingsMap) {
    // NOT USED ANY MORE
    //  Read the JSON file (assuming FileDriver::readUserFile returns nlohmann::json)
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
