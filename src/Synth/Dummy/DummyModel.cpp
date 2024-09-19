// Keep the name DummyModel (not only Model). Will be better when many tabs open..
#include "DummyModel.h"
#include "../../core/Rack.h"
#include "../../core/audio/AudioMath.h"
#include "../../utils/FNV.h"
#include "parameters.h"
#include <cmath>

constexpr float SAMPLE_RATE = 48000.0f;
constexpr float CUTOFF_FREQ = 2000.0f;

namespace Synth {
namespace Dummy {

DummyModel::DummyModel(Rack &rack)
    : rack(rack), buffer(rack.getAudioBuffer()) {
    initLPF();
    reset();
}

void DummyModel::reset() {
    // Reset any parameters if needed
}

// Runtime function to convert string to enum and call the switch logic
void DummyModel::pushParam(const std::string &name, float val) {
    auto it = paramMap.find(name); // Since we are in the same namespace, no need for Synth::Dummy::
    if (it != paramMap.end()) {
        doPushParam(it->second, val); // Call the pushParam with the found enum
    } else {
        std::cerr << "Parameter not found: " << name << "\n";
    }
}

// Push parameter
void DummyModel::doPushParam(ParamID param, float val) {
    switch (param) {
    case ParamID::Filter_freq:
        this->cutoffHz = AudioMath::logScale(val, 20.0f, 20000.0f);
        break;
    case ParamID::Pan:
        this->pan = AudioMath::clamp(val, 0.0f, 1.0f);
        break;
    }
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
        buffer[i] = AudioMath::noise() * noiseVolume;
        buffer[i + 1] = AudioMath::noise() * noiseVolume;
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
    float RC = 1.0f / (2.0f * M_PI * CUTOFF_FREQ);
    float dt = 1.0f / SAMPLE_RATE;
    lpfAlpha = dt / (RC + dt);
}

} // namespace Dummy
} // namespace Synth
