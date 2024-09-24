#include "./SynthInterface.h"
#include <iostream>

void SynthInterface::setupCCmapping(const std::string &synthName) {
    // Construct path dynamically based on the synth name
    std::string path = "Synth/" + synthName + "/cc_mappings.json";

    // Read JSON file using FileDriver
    std::string jsonData = FileDriver::readAssetFile(path);

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

void SynthInterface::handleMidiCC(int ccNumber, float value) {
    auto it = ccMappings.find(ccNumber);
    if (it != ccMappings.end()) {
        const std::string &paramName = it->second;
        pushStrParam(paramName, value); // Call the synth-specific parameter handling
    } else {
        std::cerr << "CC " << ccNumber << " not mapped." << std::endl;
    }
}
