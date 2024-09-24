#include "./DummyModel.h"
#include "core/Rack.h"

namespace Synth::Dummy {

DummyModel::DummyModel(Rack &rack)
    : rack(rack), buffer(rack.getAudioBuffer()) {
    setupParams(); // creates the array with values and lambdas for parameters
    initializeParameters();
    SynthInterface::setupCCmapping("Dummy"); // Adjust path as needed
    reset();
}

void DummyModel::reset() {
    // dunno..
}

void DummyModel::handleMidiCC(int ccNumber, float value) {
    auto it = ccMappings.find(ccNumber);
    if (it != ccMappings.end()) {
        pushStrParam(it->second, value); // Call the method to set the parameter
    } else {
        std::cerr << "CC number " << ccNumber << " not mapped!" << std::endl;
    }
}

void DummyModel::initializeParameters() {
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

void DummyModel::pushStrParam(const std::string &name, float val) {
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

bool DummyModel::pushMyParam(const std::string &name, float val) {
    std::cout << "i got here" << std::endl;
    DummyModel::pushStrParam(name, val);
    return 0;
}

void DummyModel::parseMidi(char cmd, char param1, char param2) {
    u_int8_t messageType = static_cast<uint8_t>(cmd & 0xf0);
    float fParam2 = static_cast<float>(param2) * (1.0f / 127.0f); // Normalize CC value

    switch (messageType) {
    case 0x80: // Note off
        break;
    case 0x90:             // Note on
        noiseVolume = 0.6; // Example behavior on note-on
        break;
    case 0xb0: {                                 // Control change (CC)
        int ccNumber = static_cast<int>(param1); // Get the CC number
        handleMidiCC(ccNumber, fParam2);         // Use the handler for CC mapping
        break;
    }
    default:
        // Handle other messages
        break;
    }
}

void DummyModel::initLPF() {
    float RC = 1.0f / (2.0f * M_PI * this->cutoffHz);
    float dt = 1.0f / TPH_DSP_SR;
    alpha = dt / (RC + dt);
}

void DummyModel::initBPF() {
    // Example calculation for BPF
    float RC1 = 1.0f / (2.0f * M_PI * this->cutoffHz);
    float RC2 = 1.0f / (2.0f * M_PI * (this->cutoffHz * 1.5f)); // Example bandwidth
    float dt = 1.0f / TPH_DSP_SR;
    alpha = dt / (RC1 + dt);
    // You may need to store more states for the BPF filter
}

void DummyModel::initHPF() {
    float RC = 1.0f / (2.0f * M_PI * this->cutoffHz);
    float dt = 1.0f / TPH_DSP_SR;
    alpha = RC / (RC + dt);
}

bool DummyModel::renderNextBlock() {
    for (std::size_t i = 0; i < buffer.size(); i += 2) {
        buffer[i] = AudioMath::noise() * noiseVolume * this->gainLeft;
        buffer[i + 1] = AudioMath::noise() * noiseVolume * this->gainRight;
    }

    switch (this->filterType) {
    case FilterType::LPF:
        for (std::size_t i = 0; i < buffer.size(); i += 2) {
            float leftInput = buffer[i];
            float rightInput = buffer[i + 1];
            buffer[i] = alpha * leftInput + (1.0f - alpha) * previousLeft;
            previousLeft = buffer[i];
            buffer[i + 1] = alpha * rightInput + (1.0f - alpha) * previousRight;
            previousRight = buffer[i + 1];
        }
        break;
    case FilterType::BPF:
        // Simple Band Pass Filter Implementation (can be refined)
        for (std::size_t i = 0; i < buffer.size(); i += 2) {
            float leftInput = buffer[i];
            float rightInput = buffer[i + 1];
            buffer[i] = alpha * (leftInput - previousLeft) + previousLeft;
            previousLeft = buffer[i];
            buffer[i + 1] = alpha * (rightInput - previousRight) + previousRight;
            previousRight = buffer[i + 1];
        }
        break;
    case FilterType::HPF:
        for (std::size_t i = 0; i < buffer.size(); i += 2) {
            float leftInput = buffer[i];
            float rightInput = buffer[i + 1];
            buffer[i] = alpha * (previousLeft + leftInput - buffer[i]);
            previousLeft = buffer[i];
            buffer[i + 1] = alpha * (previousRight + rightInput - buffer[i + 1]);
            previousRight = buffer[i + 1];
        }
        break;
    }
    //
    if (noiseVolume > 0.1) {
        noiseVolume -= 0.001;
    }
    return true;
}

} // namespace Synth::Dummy
