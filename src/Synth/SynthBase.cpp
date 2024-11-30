#include "SynthBase.h"
#include "core/audio/AudioMath.h"

// Define the static members - shared across instances
// This approach didn't work because lambdas need to access variables in the model.
// std::unordered_map<int, ParamDefinition> SynthInstance::paramDefs;
// std::unordered_map<std::string, int> SynthInstance::paramIndex;

void SynthBase::initParams() {
    float valToLambda;
    for (const auto &[key, def] : paramDefs) {
        paramVals[key] = def.defaultValue;
        invokeLambda(key, def);
    }
}

void SynthBase::bindBuffers(float *audioBuffer, std::size_t bufferSize) {
    this->buffer = audioBuffer;
    this->bufferSize = bufferSize;
}

void SynthBase::pushAllParams() {
    float valToLambda;
    for (const auto &[key, def] : paramDefs) {
        // paramVals[key] = def.defaultValue;
        invokeLambda(key, def);
    }
}

void SynthBase::indexParams(const int upCount) {
    paramIndex.reserve(upCount);
    for (const auto &[key, value] : SynthBase::paramDefs) {
        paramIndex[value.name] = static_cast<int>(key);
    }
}

void SynthBase::invokeLambda(const int name, const ParamDefinition &paramDef) {
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

int SynthBase::resolveUPenum(const std::string &name) {
    auto it = paramIndex.find(name); // Try to find the key
    if (it != paramIndex.end()) {
        return it->second; // Key found, return the value
    } else {
        return -1; // Return -1 if the key is not found
    }
}

void SynthBase::pushStrParam(const std::string &name, float val) {
    int upEnum = resolveUPenum(name);
    if (upEnum != -1) {
        auto it = paramDefs.find(upEnum);             // Look for the parameter in the definitions
        const ParamDefinition &paramDef = it->second; // Get the parameter definition

        float snappedVal = val; // Keep original value for non-snapped parameters
        if (paramDef.snapSteps > 0) {
            // Snap value to discrete steps
            snappedVal = round(val * (paramDef.snapSteps - 1.0f)) / (paramDef.snapSteps - 1.0f);
            // std::cout << "snapSteps:" << paramDef.snapSteps << ", snappedVal:" << snappedVal << " paramVal: " << paramVals[name] << std::endl;
            if (std::abs(snappedVal - paramVals[it->first]) < (0.9f / paramDef.snapSteps)) {
                return;
            }
        }
        // Save the normalized (0-1) value to paramVals
        // maybe omit if automation, but at same time, dsp may read these?
        if (paramDef.snapSteps > 0) {
            paramVals[it->first] = snappedVal;
        } else {
            paramVals[it->first] = val;
        }
        // now affect the dsp.
        invokeLambda(it->first, paramDef);

    } else {
        std::cerr << "Parameter not found: " << name << "\n";
    }
}

// present parameters as json, since there is no file for this in assets..
// why? Because frontend would like it.. endpoint in test.
nlohmann::json SynthBase::getParamDefsAsJson() {
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
