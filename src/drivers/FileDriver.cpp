#include "FileDriver.h"
#include "constants.h" // Include path constants
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

bool FileDriver::assetFileExists(const std::string &filename) {
    std::filesystem::path filePath = std::string(ASSETS_DIRECTORY) + "/" + filename;
    return std::filesystem::exists(filePath);
}

std::string FileDriver::assetFileRead(const std::string &filename) {
    std::ifstream file(std::string(ASSETS_DIRECTORY) + "/" + filename);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filename << " in /assets directory." << std::endl;
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

bool FileDriver::userFileExists(const std::string &filename) {
    std::filesystem::path filePath = std::string(ASSETS_DIRECTORY) + "/" + filename;
    return std::filesystem::exists(filePath);
}

std::string FileDriver::userFileRead(const std::string &filename) {
    std::ifstream file(std::string(USER_DIRECTORY) + "/" + filename);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filename << " in /user directory." << std::endl;
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    // std::cout << buffer.str() << std::endl;
    return buffer.str();
}

bool FileDriver::userFileWrite(const std::string &filename, const std::string &content) {
    try {
        // Step 1: Construct the full path to the file
        std::string fullPath = std::string(USER_DIRECTORY) + "/" + filename;

        // Step 2: Extract the directory path
        std::filesystem::path filePath(fullPath);
        std::filesystem::path dirPath = filePath.parent_path();

        // Step 3: Create the directories if they don't exist
        if (!std::filesystem::exists(dirPath)) {
            std::filesystem::create_directories(dirPath);
        }

        // Step 4: Open the file and write the content
        std::ofstream file(fullPath);
        if (!file.is_open()) {
            std::cerr << "Error opening file for writing: " << filename << " in /user directory." << std::endl;
            return false;
        }

        file << content;
        file.close();
        return true;

    } catch (const std::exception &e) {
        std::cerr << "Error: Exception occurred while writing file. Exception: " << e.what() << std::endl;
        return false;
    }
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
