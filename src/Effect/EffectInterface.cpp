#include "./EffectInterface.h"
#include <core/audio/AudioMath.h>
#include <core/ext/nlohmann/json.hpp>
#include <drivers/FileDriver.h>
#include <iostream>
// none of code below is specific to synth.
// they should be removed.

// CC-mapping stuff

void EffectInterface::setupCCmapping(const std::string &effectName) {
    // to be implemented
}

void EffectInterface::handleMidiCC(int ccNumber, float value) {
    // to be implemented
}
