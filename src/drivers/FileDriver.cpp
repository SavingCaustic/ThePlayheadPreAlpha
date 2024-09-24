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
