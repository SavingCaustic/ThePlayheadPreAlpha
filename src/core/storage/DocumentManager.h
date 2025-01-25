#pragma once
#include "drivers/FileDriver.h"
#include <iostream>
#include <map>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <vector>

namespace Storage {

// DocumentManager for JSON reading/writing
class DocumentManager {
  public:
    static nlohmann::json loadProjectFromFile(const std::string &projectName) {
        // Check if the file exists
        std::string filename = "projects/" + projectName + "/project.json";
        if (!FileDriver::assetFileExists(filename)) {
            throw std::runtime_error("File does not exist: " + filename);
        }

        // Read the file content as a string
        std::string fileContent = FileDriver::assetFileRead(filename);
        // Parse the content into JSON
        nlohmann::json json;
        try {
            json = nlohmann::json::parse(fileContent);
        } catch (const nlohmann::json::parse_error &e) {
            throw std::runtime_error("Failed to parse JSON from file: " + filename + " (" + e.what() + ")");
        }
        return json;
    }

    static nlohmann::json loadSynthPatchFromFile(const std::string &patchName, const std::string &synthType) {
        std::string filename = "Synth/" + synthType + "/patches/" + patchName + ".json";
        nlohmann::json json;
        std::cout << "trying to open " << filename << std::endl;
        if (!FileDriver::assetFileExists(filename)) {
            // bad patch name..
            return json;
        }
        std::string fileContent = FileDriver::assetFileRead(filename);
        try {
            json = nlohmann::json::parse(fileContent);
        } catch (const nlohmann::json::parse_error &e) {
            throw std::runtime_error("Failed to parse JSON from file: " + filename + " (" + e.what() + ")");
        }
        return json;
    }

    // WHAT ?!
    static void saveToFile(const nlohmann::json &json, const std::string &filename) {
        std::ofstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file: " + filename);
        }
        file << json.dump(4); // Pretty print with 4 spaces
    }
};
} // namespace Storage