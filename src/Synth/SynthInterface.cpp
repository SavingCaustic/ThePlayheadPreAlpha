#include "./SynthInterface.h"
#include <core/audio/AudioMath.h>
#include <drivers/FileDriver.h>
#include <ext/nlohmann/json.hpp>
#include <iostream>
// none of code below is specific to synth.
// they should be removed.

// Initialize the static unordered_map
std::unordered_map<std::string, ParamDefinition> SynthInterface::parameterDefinitions;

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
    // called from Rack, param not yet resolved.
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
    // when called?
    std::cout << "i got here" << std::endl;
    SynthInterface::pushStrParam(name, val);
    return 0;
}

// present parameters as json, since there is no file for this in assets..
nlohmann::json SynthInterface::getParamDefsAsJson() {
    nlohmann::json jsonOutput;
    // Iterate over parameterDefinitions and create a JSON object
    for (const auto &[paramName, paramDef] : parameterDefinitions) {
        nlohmann::json paramJson;
        paramJson["defaultValue"] = paramDef.defaultValue;
        paramJson["logCurve"] = paramDef.logCurve;
        paramJson["minValue"] = paramDef.minValue;
        paramJson["rangeFactor"] = paramDef.rangeFactor;
        paramJson["snapSteps"] = paramDef.snapSteps;
        // Add the parameter entry to the main JSON object
        jsonOutput[paramName] = paramJson;
    }
    return jsonOutput.dump(4);
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
        std::cerr << "CC " << ccNumber << " not mapped." << std::endl;
    }
}
