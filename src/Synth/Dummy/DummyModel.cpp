// Keep the name DummyModel (not only Model). Will be better when many tabs open..
#include "DummyModel.h"
#include "../../core/Rack.h"
#include "../../core/audio/AudioMath.h"
#include "../../utils/FNV.h"
#include <cmath>

constexpr float CUTOFF_FREQ = 2000.0f;

namespace Synth::Dummy {

DummyModel::DummyModel(Rack &rack)
    : rack(rack), buffer(rack.getAudioBuffer()) {
    setupParams();
    initializeParameters();
    // initLPF();
    reset();
}

void DummyModel::reset() {
    // Reset any parameters if needed
}

void DummyModel::initLPF() {
    float RC = 1.0f / (2.0f * M_PI * this->cutoffHz);
    float dt = 1.0f / TPH_DSP_SR;
    lpfAlpha = dt / (RC + dt);
}

void DummyModel::initializeParameters() {
    for (const auto &[key, definition] : parameterDefinitions) {
        if (definition.transformFunc) {
            definition.transformFunc(definition.defaultValue);
        }
    }
}

void DummyModel::pushStrParam(const std::string &name, float val) {
    std::cout << "dealing with " << &name << " and its new value " << val << std::endl;
    auto it = parameterDefinitions.find(name); // Look for the parameter in the definitions
    if (it != parameterDefinitions.end()) {
        const ParamDefinition &paramDef = it->second; // Get the parameter definition
        // Check if a callback function exists and call it with the provided value
        if (paramDef.transformFunc) {
            paramDef.transformFunc(val); // Call the lambda
        } else {
            std::cerr << "No callback function for parameter: " << name << "\n";
        }
    } else {
        std::cerr << "Parameter not found: " << name << "\n";
    }
}

bool DummyModel::pushMyParam(const std::string &name, float val) {
    std::cout << "i got here" << std::endl;
    DummyModel::pushStrParam(name, val);
    return 0;
}

void DummyModel::parseMidi(char cmd, char param1, char param2) {
    u_int8_t test = static_cast<uint8_t>(cmd & 0xf0);

    switch (test) {
    case 0x80:
        // note off
        break;
    case 0x90:
        // note on
        noiseVolume = 0.6;
    }
}

bool DummyModel::renderNextBlock() {
    for (std::size_t i = 0; i < buffer.size(); i += 2) {
        buffer[i] = AudioMath::noise() * noiseVolume * this->gainLeft;
        buffer[i + 1] = AudioMath::noise() * noiseVolume * this->gainRight;
    }

    for (std::size_t i = 0; i < buffer.size(); i += 2) {
        float leftInput = buffer[i];
        float rightInput = buffer[i + 1];

        buffer[i] = lpfAlpha * leftInput + (1.0f - lpfAlpha) * lpfPreviousLeft;
        lpfPreviousLeft = buffer[i];

        buffer[i + 1] = lpfAlpha * rightInput + (1.0f - lpfAlpha) * lpfPreviousRight;
        lpfPreviousRight = buffer[i + 1];
    }

    if (noiseVolume > 0.1) {
        noiseVolume -= 0.001;
    }
    return true;
}

} // namespace Synth::Dummy
