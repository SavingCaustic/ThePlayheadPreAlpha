// Keep the name DummyModel (not only Model). Will be better when many tabs open..
#include "DummyModel.h"
#include "../../core/Rack.h"
#include "../../core/audio/AudioMath.h"
#include "../../utils/FNV.h"
#include "parameters.h"
#include <cmath>

constexpr float CUTOFF_FREQ = 2000.0f;

namespace Synth::Dummy {

DummyModel::DummyModel(Rack &rack)
    : rack(rack), buffer(rack.getAudioBuffer()) {
    initLPF();
    reset();
}

void DummyModel::reset() {
    // Reset any parameters if needed
}

// Runtime function to convert string to enum and call the switch logic
void DummyModel::pushStrParam(const std::string &name, int val) {
    auto it = paramMap.find(name); // Since we are in the same namespace, no need for Synth::Dummy::
    if (it != paramMap.end()) {
        doPushParam(it->second, val); // Call the pushParam with the found enum
    } else {
        std::cerr << "Parameter not found: " << name << "\n";
    }
}

// Push parameter
void DummyModel::doPushParam(ParamID param, int val) {
    float decVal = val / 127.0f;
    switch (param) {
    case ParamID::Filter_freq:
        std::cout << "filter_freq" << std::endl;
        this->cutoffHz = AudioMath::logScale2(val, 2000, 16);
        this->initLPF();
        break;
    case ParamID::Pan:
        //  Use sin/cos panning law, adjusting the input to map to 0-1 range for csin/ccos
        this->gainLeft = AudioMath::ccos(decVal * 0.25f);  // Left gain decreases as panVal goes to 1
        this->gainRight = AudioMath::csin(decVal * 0.25f); // Right gain increases as panVal goes to 1
        std::cout << "setting gain to (L,R)" << this->gainLeft << ", " << this->gainRight << std::endl;
        break;
    default:
        std::cout << "failure" << std::endl;
        break;
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

void DummyModel::initLPF() {
    float RC = 1.0f / (2.0f * M_PI * this->cutoffHz);
    float dt = 1.0f / TPH_DSP_SR;
    lpfAlpha = dt / (RC + dt);
}

} // namespace Synth::Dummy
