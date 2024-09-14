#include "DummyModel.h"
#include "../../core/Rack.h"

// Constructor
DummyModel::DummyModel(Rack &rack)
    : rack(rack), buffer(rack.getAudioBuffer()) {
    reset();
}

// Reset method
void DummyModel::reset() {
    // Reset any parameters if needed
}

// Set parameters
void DummyModel::setParam(const std::string &name, float val) {
    // Implement parameter setting logic if needed
}

// Parse MIDI commands
void DummyModel::parseMidi(int cmd, int param1, int param2) {
    // Implement MIDI parsing logic if needed
    char cmdMSN = cmd && 0xf0;
    switch (cmdMSN) {
    case 0x80:
        // note off
    case 0x90:
        // note on
        noiseVolume = 0.6;
    }
}

// Render the next block of audio
bool DummyModel::renderNextBlock() {
    for (std::size_t i = 0; i < buffer.size(); i += 2) {
        buffer[i] = noiseVolume * (static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f);           // Left channel
        buffer[i + 1] = 0.2 * noiseVolume * (static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f); // Right channel
    }
    if (noiseVolume > 0.2) {
        noiseVolume -= 0.001;
    }
    return true; // stereo-signal
}
