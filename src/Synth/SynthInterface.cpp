#include "./SynthInterface.h"
#include <core/audio/AudioMath.h>
#include <core/ext/nlohmann/json.hpp>
#include <drivers/FileDriver.h>
#include <iostream>

// Initialize the static unordered_map
std::unordered_map<std::string, ParamDefinition> SynthInterface::paramDefs;

void SynthInterface::initializeParameters() {
    float valToLambda;
    for (const auto &[key, def] : paramDefs) {
        invokeLambda(key, def);
    }
}

// Struct for parameter definitions
/*
struct ParamDefinition {
    float defaultValue;                       // Default value (e.g., 0-1 for continuous values)
    int snapSteps;                            // Snap-steps (e.g., semitone or octave selection)
    bool logCurve;                            // Logarithmic curve if true, linear if false
    int minValue;                             // Minimum value
    int rangeFactor;                          // Factor for scaling (based on log or linear curve)
    std::function<void(float)> transformFunc; // Lambda for handling parameter changes
};
*/

void SynthInterface::invokeLambda(const std::string &name, const ParamDefinition &paramDef) {
    float valToLambda;
    float val;
    val = paramVals[name];
    if (paramDef.logCurve) {
        // do log stuff
        valToLambda = AudioMath::logScale(val, paramDef.minValue, paramDef.rangeFactor);
    } else {
        // do lin stuff. Snaps will also go here - from float to int-ish..
        valToLambda = AudioMath::linScale(val, paramDef.minValue, paramDef.rangeFactor);
        if (paramDef.snapSteps > 0) {
            valToLambda = round(valToLambda);
        }
    }

    // Check if a callback function exists and call it with the provided value
    if (paramDef.transformFunc) {
        paramDef.transformFunc(valToLambda); // Call the lambda
    } else {
        std::cerr << "No callback function for parameter: " << name << "\n";
    }
}

void SynthInterface::pushStrParam(const std::string &name, float val) {
    auto it = paramDefs.find(name); // Look for the parameter in the definitions
    if (it != paramDefs.end()) {
        const ParamDefinition &paramDef = it->second; // Get the parameter definition

        float snappedVal = val; // Keep original value for non-snapped parameters
        if (paramDef.snapSteps > 0) {
            // Snap value to discrete steps
            snappedVal = round(val * (paramDef.snapSteps - 1.0f)) / (paramDef.snapSteps - 1.0f);
            // std::cout << "snapSteps:" << paramDef.snapSteps << ", snappedVal:" << snappedVal << " paramVal: " << paramVals[name] << std::endl;
            if (std::abs(snappedVal - paramVals[name]) < (0.9f / paramDef.snapSteps)) {
                return;
            }
        }
        // Save the normalized (0-1) value to paramVals
        // maybe omit if automation, but at same time, dsp may read these?
        if (paramDef.snapSteps > 0) {
            paramVals[name] = snappedVal;
        } else {
            paramVals[name] = val;
        }
        // now affect the dsp.
        invokeLambda(name, paramDef);

    } else {
        std::cerr << "Parameter not found: " << name << "\n";
    }
}

bool SynthInterface::pushMyParam(const std::string &name, float val) {
    // when called?
    std::cout << "i got here" << std::endl;
    SynthInterface::pushStrParam(name, val);
    return 0;
}

// present parameters as json, since there is no file for this in assets..
// why? Because frontend would like it.. endpoint in test.
nlohmann::json SynthInterface::getParamDefsAsJson() {
    nlohmann::json jsonOutput;
    // Iterate over parameterDefinitions and create a JSON object
    for (const auto &[paramName, paramDef] : paramDefs) {
        nlohmann::json paramJson;
        paramJson["defaultValue"] = paramDef.defaultValue;
        paramJson["logCurve"] = paramDef.logCurve;
        paramJson["minValue"] = paramDef.minValue;
        paramJson["rangeFactor"] = paramDef.rangeFactor;
        paramJson["snapSteps"] = paramDef.snapSteps;
        // Add the parameter entry to the main JSON object
        jsonOutput[paramName] = paramJson;
    }
    return jsonOutput.dump(4); //?? huh??
}

// CC-mapping stuff

void SynthInterface::setupCCmapping(const std::string &synthName) {
    // setup for each rack right, event if same synth..
    // Construct path dynamically based on the synth name
    std::string path = "Synth/" + synthName + "/cc_mappings.json";
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

void SynthInterface::handleMidiCC(u_int8_t ccNumber, float value) {
    auto it = ccMappings.find(ccNumber);
    if (it != ccMappings.end()) {
        const std::string &paramName = it->second;
        pushStrParam(paramName, value); // Call the synth-specific parameter handling
    } else {
        std::cerr << "CC " << static_cast<int>(ccNumber) << " not mapped." << std::endl;
    }
}
