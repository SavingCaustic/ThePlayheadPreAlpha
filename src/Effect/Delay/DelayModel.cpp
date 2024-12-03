#include "./DelayModel.h"
// #include "core/Rack.h"
// #include "ext/dr_wav.h" //not used but now we have it..
// #include <iostream>
#include <vector>

namespace Effect::Delay {

// Constructor to accept buffer and size
Model::Model(float *audioBuffer, std::size_t bufferSize)
    : buffer(audioBuffer), bufferSize(bufferSize) {
    setupParams();                 // creates the array with attributes and lambdas for parameters - NOT INTERFACE
    delayBuffer.resize(48000 * 2); // maybe later times 2..
    std::fill(delayBuffer.begin(), delayBuffer.end(), 0.0f);
    // EffectInterface::initializeParameters();
    //  EffectInterface::setupCCmapping("Dummy"); // Adjust path as needed
    reset();
}

void Model::reset() {
    // must be safe to call inside audio-thread, right?
    wrPointer = time * 48000;
    rdPointer = 0;
}

void Model::setupParams() {
}

void Model::parseMidi(char cmd, char param1, char param2) {
    u_int8_t messageType = static_cast<uint8_t>(cmd & 0xf0);
    float fParam2 = static_cast<float>(param2) * (1.0f / 127.0f);

    switch (messageType) {
    case 0x80:
        // Note off
        break;
    case 0x90:
        // Note on
        break;
    case 0xb0:
        // Control change (CC)
        EffectInterface::handleMidiCC(static_cast<int>(param1), fParam2);
        break;
    default:
        // Handle other messages
        break;
    }
}

bool Model::renderNextBlock(bool isSterero) {
    // 1) get the delayOut-signal
    // 2) get the audioIn-signal
    // 3) write the delayIn signal based on feedback
    // 4) write the audioOut signal based on mix
    // ok, stereo..
    //
    float delayOutL, delayOutR;
    float audioInL, audioInR;
    for (std::size_t i = 0; i < bufferSize; i += 2) {
        //
        delayOutL = delayBuffer[rdPointer];
        delayOutR = delayBuffer[rdPointer + 1];
        //
        audioInL = buffer[i];
        audioInR = buffer[i + 1];
        //
        delayBuffer[wrPointer] = audioInL * (1 - feedback) + delayOutL * (feedback);
        delayBuffer[wrPointer + 1] = audioInR * (1 - feedback) + delayOutR * (feedback);
        //
        buffer[i] = delayOutL * mix + audioInL * (1 - mix);
        buffer[i + 1] = delayOutR * mix + audioInR * (1 - mix);
        //
        wrPointer = (wrPointer + 2) % 96000;
        rdPointer = (rdPointer + 2) % 96000;
    }

    //
    return true;
}

} // namespace Effect::Delay
