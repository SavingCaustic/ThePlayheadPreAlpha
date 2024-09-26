#include "FileDriver.h"
#include "constants.h" // Include path constants
#include <fstream>
#include <iostream>
#include <sstream>

std::string FileDriver::readAssetFile(const std::string &filename) {
    std::ifstream file(std::string(ASSETS_DIRECTORY) + "/" + filename);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filename << " in /assets directory." << std::endl;
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::string FileDriver::readUserFile(const std::string &filename) {
    std::ifstream file(std::string(USER_DIRECTORY) + "/" + filename);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filename << " in /user directory." << std::endl;
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

bool FileDriver::writeUserFile(const std::string &filename, const std::string &content) {
    std::ofstream file(std::string(USER_DIRECTORY) + "/" + filename);
    if (!file.is_open()) {
        std::cerr << "Error opening file for writing: " << filename << " in /user directory." << std::endl;
        return false;
    }

    file << content;
    if (!file) {
        std::cerr << "Error writing to file: " << filename << std::endl;
        return false;
    }

    return true;
}

// these was in main..
bool FileDriver::ends_with(const std::string &str, const std::string &suffix) {
    return str.size() >= suffix.size() &&
           str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

std::string FileDriver::load_file_content(const std::string &filepath) {
    std::ifstream file(filepath);

    if (!file) {
        return ""; // Return empty string if file not found
    }

    std::stringstream buffer;
    buffer << file.rdbuf(); // Load the whole file into the stringstream
    return buffer.str();
}
