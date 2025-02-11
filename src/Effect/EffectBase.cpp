#include "EffectBase.h"
#include "core/audio/AudioMath.h"

// Define the static members - shared across instances
// This approach didn't work because lambdas need to access variables in the model.
// std::unordered_map<int, ParamDefinition> SynthInstance::paramDefs;
// std::unordered_map<std::string, int> SynthInstance::paramIndex;

void EffectBase::initParams() {
    float valToLambda;
    for (const auto &[key, def] : paramDefs) {
        paramVals[key] = def.defaultValue;
        invokeLambda(key, def);
    }
}

void EffectBase::bindBuffers(float *audioBufferLeft, float *audioBufferRight, std::size_t bufferSize) {
    std::cout << "binding buffersize to " << bufferSize << std::endl;
    this->bufferLeft = audioBufferLeft;
    this->bufferRight = audioBufferRight;
    this->bufferSize = bufferSize;
}

void EffectBase::pushAllParams() {
    float valToLambda;
    for (const auto &[key, def] : paramDefs) {
        // paramVals[key] = def.defaultValue;
        invokeLambda(key, def);
    }
}

void EffectBase::indexParams(const int upCount) {
    paramIndex.reserve(upCount);
    for (const auto &[key, value] : EffectBase::paramDefs) {
        paramIndex[value.name] = static_cast<int>(key);
    }
}

void EffectBase::invokeLambda(const int name, const ParamDefinition &paramDef) {
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

int EffectBase::resolveUPenum(const std::string &name) {
    auto it = paramIndex.find(name); // Try to find the key
    if (it != paramIndex.end()) {
        return it->second; // Key found, return the value
    } else {
        return -1; // Return -1 if the key is not found
    }
}

std::string EffectBase::resolveUPname(const int paramID) {
    // Check if the parameter ID exists in paramDefs
    auto it = paramDefs.find(paramID);
    if (it != paramDefs.end()) {
        // Return the name field of the parameter definition
        return it->second.name;
    }

    // If not found, return an empty string
    return "";
}

void EffectBase::pushStrParam(const std::string &name, float val) {
    // here "name" is actually 1-6. Nothing else..
    // std::cout << "running pustStrParam with name: " << name << std::endl;
    int upEnum = stoi(name); // resolveUPenum(name); // this should be replaced as we pass a number in the name when effects. (Right?)
    // reason we do that is because it's the rack who's performing midi action, not knowing stuff about effect.
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
nlohmann::json EffectBase::getParamDefsAsJson() {
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

// void EffectBase::setupCCmapping(const std::string &effectName) {
// the cc-mapping it's actually in the rack, not the effect..
//  i'd rather not have json files here. just use enum of parameters..
//}

// void EffectBase::handleMidiCC(int ccNumber, float value) {
//  maybe the cc's should be changed *outside* the effect and not here..
//  yeah kind of makes more sense. it's rack how's manipulating the effect from outside..
//  so effects does *not* recieve midi. Settled!
//}
