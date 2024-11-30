#pragma once
#include "Synth/Monolith/MonolithModel.h"
#include <Synth/SynthBase.h>
#include <drivers/FileDriver.h>
#include <nlohmann/json.hpp> // Include the JSON library
// #include "Synth/Sketch/SketchModel.h"
// #include "Synth/Subreal/SubrealModel.h"

using json = nlohmann::json;

enum class SynthType {
    Monolith,
    Subreal,
    Sketch,
    // Add other synth types here
    Unknown
};

class SynthFactory {
  public:
    static SynthType getSynthType(const std::string &synthName) {
        // i really don't know what i need this for..
        if (synthName == "Monolith")
            return SynthType::Monolith;
        if (synthName == "Subreal")
            return SynthType::Subreal;
        if (synthName == "Sketch")
            return SynthType::Sketch;
        // Add other synth type checks here
        return SynthType::Unknown;
    }

    static bool setupSynth(SynthBase *&newSynth, const std::string &synthName) {
        std::cout << "we're setting up synth: " << synthName << std::endl;
        SynthType type = getSynthType(synthName);

        // Clean up old synth if any, should probably not be done here..
        if (newSynth) {
            delete newSynth;
            newSynth = nullptr;
        }

        bool loadOK = true;
        switch (type) {
        case SynthType::Monolith:
            newSynth = new Synth::Monolith::Model(); // audioBuffer.data(), audioBuffer.size());
            break;
        case SynthType::Subreal:
            // synth = new Synth::Subreal::Model(audioBuffer.data(), audioBuffer.size());
            break;
        case SynthType::Sketch:
            // synth = new Synth::Sketch::Model(audioBuffer.data(), audioBuffer.size());
            break;
        // Add cases for other synth types here
        default:
            std::cerr << "Unknown synth type: " << synthName << std::endl;
            return false; // Indicate failure
            loadOK = false;
        }
        return true;
        /*if (loadOK) {
            synth->setErrorWriter(errorWriter_);
            this->enabled = true;
        }
        return (loadOK);*/
    }

    static bool patchLoad(SynthBase *&synth, std::string patchName) {
        // step 0: start out with default values:
        synth->initParams();

        // Step 1: Get the file from the file system
        std::string s = FileDriver::readAssetFile("/Synth/Monolith/Patches/" + patchName + ".json");
        if (s.empty()) {
            std::cerr << "Error: Patch file not found or empty." << std::endl;
            return false;
        }

        try {
            // Step 2: Parse the JSON string
            json patchData = json::parse(s);

            // Step 3: Check if "params" exists and is an object
            if (!patchData.contains("params") || !patchData["params"].is_object()) {
                std::cerr << "Error: Invalid patch format (missing 'params')." << std::endl;
                return false;
            }

            // Step 4: Iterate over the key-value pairs in "params"
            for (const auto &[key, value] : patchData["params"].items()) {
                // Ensure value is a number (or convert if necessary)
                if (!value.is_number()) {
                    std::cerr << "Warning: Parameter " << key << " has a non-numeric value. Skipping." << std::endl;
                    continue;
                }

                float paramValue = value.get<float>();

                // Step 5: Resolve the parameter name to its enum/int value
                int paramEnum = synth->resolveUPenum(key);
                if (paramEnum == -1) {
                    std::cerr << "Warning: Parameter " << key << " not recognized. Skipping." << std::endl;
                    continue;
                }

                // Step 6: Update the parameter value in paramVals
                synth->paramVals[paramEnum] = paramValue;
            }
            // now push all params to dsp.
            synth->pushAllParams();

            std::cout << "Patch loaded successfully." << std::endl;
            return true;

        } catch (const std::exception &e) {
            std::cerr << "Error: Failed to parse patch file. Exception: " << e.what() << std::endl;
            return false;
        }
    }
};