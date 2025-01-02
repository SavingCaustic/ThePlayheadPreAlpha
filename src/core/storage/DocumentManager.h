#pragma once
#include <fstream>
#include <iostream>
#include <map>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace Storage {

// DocumentManager for JSON reading/writing
class DocumentManager {
  public:
    static nlohmann::json loadFromFile(const std::string &filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file: " + filename);
        }
        nlohmann::json json;
        file >> json;
        return json;
    }

    static void saveToFile(const nlohmann::json &json, const std::string &filename) {
        std::ofstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file: " + filename);
        }
        file << json.dump(4); // Pretty print with 4 spaces
    }
};
} // namespace Storage