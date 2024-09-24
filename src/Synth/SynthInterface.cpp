#include "./SynthInterface.h"
#include <core/audio/AudioMath.h>
#include <iostream>

void SynthInterface::initializeParameters() {
    float valToLambda;
    for (const auto &[key, def] : parameterDefinitions) {
        if (def.transformFunc) {
            // not so fast, we need to use the right conversion
            if (def.logCurve) {
                // do log stuff
                valToLambda = AudioMath::logScale(def.defaultValue, def.minValue, def.rangeFactor);
            } else {
                // do lin stuff
                valToLambda = AudioMath::linScale(def.defaultValue, def.minValue, def.rangeFactor);
            } // and what about snaps and enums?
            def.transformFunc(valToLambda);
        }
    }
}

void SynthInterface::pushStrParam(const std::string &name, float val) {
    std::cout << "dealing with " << &name << " and its new value " << val << std::endl;
    auto it = parameterDefinitions.find(name); // Look for the parameter in the definitions
    float valToLambda;
    if (it != parameterDefinitions.end()) {
        const ParamDefinition &paramDef = it->second; // Get the parameter definition
        // transform value here and let lambda focus on setting registers.
        if (paramDef.logCurve) {
            // do log stuff
            valToLambda = AudioMath::logScale(val, paramDef.minValue, paramDef.rangeFactor);
        } else {
            // do lin stuff
            valToLambda = AudioMath::linScale(val, paramDef.minValue, paramDef.rangeFactor);
        } // and what about snaps and enums?

        // Check if a callback function exists and call it with the provided value
        if (paramDef.transformFunc) {
            paramDef.transformFunc(valToLambda); // Call the lambda
        } else {
            std::cerr << "No callback function for parameter: " << name << "\n";
        }
    } else {
        std::cerr << "Parameter not found: " << name << "\n";
    }
}

bool SynthInterface::pushMyParam(const std::string &name, float val) {
    std::cout << "i got here" << std::endl;
    SynthInterface::pushStrParam(name, val);
    return 0;
}

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
