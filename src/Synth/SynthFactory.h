#pragma once
#include <Synth/SynthInstance.h>
#include <drivers/FileDriver.h>

#include "Synth/Monolith/MonolithModel.h"
// #include "Synth/Sketch/SketchModel.h"
// #include "Synth/Subreal/SubrealModel.h"

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

    static bool setupSynth(SynthInstance *&newSynth, const std::string &synthName) {
        std::cout << "we're setting up synth: " << synthName << std::endl;
        SynthType type = getSynthType(synthName);

        // Clean up old synth if any
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

    static bool patchLoad(SynthInstance *&synth, std::string patchName) {
        // get the file from file system..
        std::string s = FileDriver::readAssetFile("/Synth/Monolith/Patches/" + patchName + ".json");
        std::cout << s << std::endl;
        // load the params and settings

        // first run settings?

        // run init for params.
        return true;
    }
};