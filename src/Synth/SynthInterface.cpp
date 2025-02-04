#include "./SynthInterface.h"
#include <core/audio/AudioMath.h>
#include <core/ext/nlohmann/json.hpp>
#include <drivers/FileDriver.h>
#include <iostream>

// Initialize the static unordered_map

void SynthInterface::logErr(int code, const std::string &message) {
    std::cerr << "std:Error [" << code << "]: " << message << std::endl;
}

// CC-mapping stuff

void SynthInterface::setupCCmapping(const std::string &synthName) {
    std::string path = "Synth/" + synthName + "/cc_mappings.json";
    std::cout << "cc-mapping setting up on " << path << std::endl;

    std::string jsonData = FileDriver::assetFileRead(path);
    if (jsonData.empty()) {
        std::cerr << "Failed to read CC mapping file for " << synthName << std::endl;
        return;
    }

    try {
        auto ccMappingsJson = nlohmann::json::parse(jsonData);
        for (const auto &[cc, param] : ccMappingsJson.items()) {
            int ccNumber = std::stoi(cc);
            std::string paramName = param.get<std::string>();

            // Ensure it fits in the buffer
            std::array<char, 16> nameBuffer = {};
            std::strncpy(nameBuffer.data(), paramName.c_str(), nameBuffer.size() - 1);
            nameBuffer[nameBuffer.size() - 1] = '\0'; // Null-terminate

            ccMappings[ccNumber] = nameBuffer; // Store the fixed-size array
        }
    } catch (const nlohmann::json::parse_error &e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
    }
}

void SynthInterface::handleMidiCC(uint8_t ccNumber, float value) {
    auto it = ccMappings.find(ccNumber);
    if (it != ccMappings.end()) {
        pushStrParam(it->second.data(), value); // Call the synth-specific parameter handling
    } else {
        std::cerr << "CC " << static_cast<int>(ccNumber) << " not mapped." << std::endl;
    }
}
