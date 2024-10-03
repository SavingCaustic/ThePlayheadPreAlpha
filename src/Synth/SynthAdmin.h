#include <SynthInterface.h>
#include <core/params.h>
#include <drivers/FileDriver.h>
#include <ext/nlohmann/json.hpp>
#include <iostream>

// here all stuff that isn't deterministic should be here..

class SynthAdmin {
  public:
    SynthAdmin(SynthInterface &model) : synthInterface(synthInterface) {}
    // somewhere here have the actual parameters - not only read only definitions..
    // that's what's being loaded and saved.
    // parameter-changes affects these too, so we must be given permission from playerEngine..
    bool loadPatch() {}
    bool savePatch() {}
    std::string formatParametersAsString() {}

    // present parameters as json, since there is no file for this in assets..
    nlohmann::json getParamDefsAsJson() {
        nlohmann::json jsonOutput;
        // Iterate over parameterDefinitions and create a JSON object. Not working right..
        /*
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
        */
        return jsonOutput.dump(4);
    }

    void setupCCmapping(const std::string &synthName) {
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

  private:
    SynthInterface &synthInterface; // Reference to the synth interface
};