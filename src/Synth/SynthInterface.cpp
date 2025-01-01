#include "./SynthInterface.h"
#include "core/player/ErrorWriter.h"
#include <core/audio/AudioMath.h>
#include <core/ext/nlohmann/json.hpp>
#include <drivers/FileDriver.h>
#include <iostream>

// Initialize the static unordered_map

void SynthInterface::logErr(int code, const std::string &message) {
    if (errorWriter_) {
        errorWriter_->logError(code, message);
    } else {
        std::cerr << "std:Error [" << code << "]: " << message << std::endl;
    }
}

// CC-mapping stuff

void SynthInterface::setupCCmapping(const std::string &synthName) {
    // setup for each rack right, event if same synth..
    // Construct path dynamically based on the synth name
    std::string path = "Synth/" + synthName + "/cc_mappings.json";
    std::cout << "cc-mapping setting up on " << path << std::endl;
    std::string jsonData = FileDriver::assetFileRead(path);
    if (jsonData.empty()) {
        std::cerr << "Failed to read CC mapping file for " << synthName << std::endl;
        return;
    }
    // Parse the JSON data
    try {
        auto ccMappingsJson = nlohmann::json::parse(jsonData);
        // Populate the ccMappings map
        for (const auto &[cc, param] : ccMappingsJson.items()) {
            int ccNumber = std::stoi(cc);
            ccMappings[ccNumber] = param.get<std::string>();
        }
    } catch (const nlohmann::json::parse_error &e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
    }
}

void SynthInterface::handleMidiCC(uint8_t ccNumber, float value) {
    auto it = ccMappings.find(ccNumber);
    if (it != ccMappings.end()) {
        const std::string &paramName = it->second;
        pushStrParam(paramName, value); // Call the synth-specific parameter handling
    } else {
        std::cerr << "CC " << static_cast<int>(ccNumber) << " not mapped." << std::endl;
    }
}
