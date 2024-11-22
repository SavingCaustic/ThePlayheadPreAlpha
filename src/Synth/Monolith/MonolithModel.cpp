#include "./MonolithModel.h"
#include "core/audio/envelope/ADSFR.h"
#include "core/audio/filter/MultiFilter.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <vector>

namespace Synth::Monolith {

// hmm - how should frequencies be calculated?
// keyboard suggests note, like midi.
// pitchbend changes suggested note.
// oscillators have their *factors* based on range and detune. +/-7 detune with 127 steps give = 8.5 steps per semitone.
// if factor is 0.5 + (0-1) we get 0.5 =>
// too little. So we probably need a coarse and fine detune for at least osc2.
//

// Constructor to accept buffer and size
Model::Model(float *audioBuffer, std::size_t bufferSize)
    : buffer(audioBuffer), bufferSize(bufferSize) {
    setupParams(); // creates the array with attributes and lambdas for parameters - NOT INTERFACE
    SynthInterface::initializeParameters();
    SynthInterface::setupCCmapping("Monolith");
    reset();
}

void Model::setupParams() {
    if (SynthInterface::paramDefs.empty()) {
        SynthInterface::paramDefs = {
            {"kbd_glide", {0.5f, 0, true, 1, 10, [this](float v) {
                               portamentoAlpha = 1 - (1.0f / v);
                               // trust the interface to resolve sendError
                               this->sendError("changing kbd_glide..");
                           }}},
            {"osc1_detune", {0.5f, 0, false, -2, 2, [this](float v) {
                                 osc1detune = round(v);
                             }}},
            {"osc1_range", {0.5f, 6, false, 0, 5, [this](float v) {
                                osc1rangeFactor = std::exp2(round(v)) * 0.125f;
                                // special case if lowest, go one extra down..
                                if (v == 0)
                                    osc1rangeFactor *= 0.5f;
                            }}},
            {"osc1_wf", {0.4f, 6, false, 0, 5, [this](float v) {
                             std::cout << "setting new waveform to " << v << std::endl;
                             osc1wf = static_cast<Waveform>(static_cast<int>(v));
                         }}},
            {"osc1_vol", {0.7f, 0, true, 10, 8, [this](float v) {
                              // it's probably here that we should have the easer. meh...
                              osc1vol = (v - 10) * (1.0f / 2550.0f);
                          }}},
            {"osc2_detune", {0.45f, 0, false, -1, 1, [this](float v) {
                                 osc2detune = (v < 0) ? -pow(fabs(v * 3.7f), 1.5) : pow(v * 3.7f, 1.5);
                             }}},
            {"osc2_range", {0.5f, 6, false, 0, 5, [this](float v) {
                                osc2rangeFactor = std::exp2(round(v)) * 0.125f;
                                // special case if lowest, go one extra down..
                                if (v == 0)
                                    osc2rangeFactor *= 0.5f;
                            }}},
            {"osc2_wf", {0.0f, 6, false, 0, 5, [this](float v) {
                             std::cout << "setting new waveform to " << v << std::endl;
                             osc2wf = static_cast<Waveform>(static_cast<int>(v));
                         }}},
            {"osc2_vol", {0.7f, 0, true, 10, 8, [this](float v) {
                              // it's probably here that we should have the easer. meh...
                              osc2vol = (v - 10) * (1.0f / 2550.0f);
                          }}},
            {"osc3_detune", {0.52f, 0, false, -1, 1, [this](float v) {
                                 osc3detune = (v < 0) ? -pow(fabs(v * 3.7f), 1.5) : pow(v * 3.7f, 1.5);
                             }}},
            {"osc3_range", {0.5f, 6, false, 0, 5, [this](float v) {
                                osc3rangeFactor = std::exp2(round(v)) * 0.125f;
                                // special case if lowest, go one extra down..
                                if (v == 0)
                                    osc3rangeFactor *= 0.5f;
                            }}},
            {"osc3_wf", {0.0f, 6, false, 0, 5, [this](float v) {
                             std::cout << "setting new waveform to " << v << std::endl;
                             osc3wf = static_cast<Waveform>(static_cast<int>(v));
                             if (osc3wf == KNEANGLE)
                                 osc3wf = TOOTHSAW;
                         }}},
            {"osc3_vol", {0.7f, 0, true, 10, 8, [this](float v) {
                              // it's probably here that we should have the easer. meh...
                              osc3vol = (v - 10) * (1.0f / 2550.0f);
                          }}},
            {"vca_attack", {0.0f, 0, true, 2, 11, [this](float v) { // 8192 max
                                vcaAR.setTime(audio::envelope::ATTACK, v);
                            }}},
            {"vca_decay", {0.5f, 0, true, 5, 7, [this](float v) { // 8192 max
                               vcaAR.setTime(audio::envelope::DECAY, v);
                           }}},
            {"vca_sustain", {0.7f, 0, false, 0, 1, [this](float v) {
                                 vcaAR.setLevel(audio::envelope::SUSTAIN, v);
                             }}},
            {"vca_fade", {0.0f, 0, false, 0, 1, [this](float v) {
                              vcaAR.setLeak(audio::envelope::FADE, v);
                          }}},
            {"vca_release", {0.1f, 0, true, 10, 8, [this](float v) {
                                 vcaAR.setTime(audio::envelope::RELEASE, v);
                             }}},
            {"lfo1_speed", {0.0f, 0, true, 100, 9, [this](float v) {
                                lfo1.setSpeed(v); // in mHz.
                            }}}};
    }
}

void Model::reset() {
}

void Model::parseMidi(uint8_t cmd, uint8_t param1, uint8_t param2) {
    uint8_t messageType = cmd & 0xf0;
    float fParam2 = static_cast<float>(param2) * (1.0f / 127.0f);
    KeyAction keyAction;
    switch (messageType) {
    case 0x90:
        keyAction = pressKey(param1);
        switch (keyAction) {
        case TRIGGER:
            // accepted
            notePlaying = param1;
            fNotePlaying = static_cast<float>(param1); // skip portamento
            noteVelocity = fParam2 + 0.1f;
            // calculate inv. freq to avoid division later.
            vcaAR.triggerSlope(vcaARslope, audio::envelope::NOTE_ON);
            break;
        case NEW_NOTE:
            notePlaying = param1;
            noteVelocity = fParam2 + 0.1f;
            // calculate inv. freq to avoid division later.
            break;
        }
        break;
    case 0x80:
        keyAction = releaseKey(param1);
        switch (keyAction) {
        case TRIGGER:
            vcaAR.triggerSlope(vcaARslope, audio::envelope::NOTE_OFF);
            break;
        case NEW_NOTE:
            notePlaying = keysPressed[0];
            break;
        }
        break;
    case 0xb0:
        SynthInterface::handleMidiCC(param1, fParam2);
        break;
    case 0xe0: {
        int pitchBendValue = (static_cast<int>(param2) << 7) | static_cast<int>(param1);
        bendSemis = ((pitchBendValue - 8192) / 8192.0f) * 2.0f;
        break;
    }
    }
}

KeyAction Model::pressKey(uint8_t key) {
    // Check if the key is already in the array
    for (int i = 0; i < maxKeys; ++i) {
        if (keysPressed[i] == key) {
            return NO_ACTION; // Key is already pressed
        }
    }
    // Find the first empty slot (0)
    int emptyIndex = -1;
    for (int i = 0; i < maxKeys; ++i) {
        if (keysPressed[i] == 0) {
            emptyIndex = i;
            break;
        }
    }
    if (emptyIndex != -1) {
        // Shift all keys up to make room at index 0
        for (int i = emptyIndex; i > 0; --i) {
            keysPressed[i] = keysPressed[i - 1];
        }
        keysPressed[0] = key;
        if (emptyIndex == 0) {
            return TRIGGER;
        } else {
            return NEW_NOTE;
        }
    }

    // All slots are full
    return NO_ACTION;
}

KeyAction Model::releaseKey(uint8_t key) {
    int index = -1;
    for (int i = 0; i < maxKeys; ++i) {
        if (keysPressed[i] == key) {
            index = i;
            break;
        }
    }

    if (index != -1) {
        // Shift remaining keys down
        for (int i = index; i < maxKeys - 1; ++i) {
            keysPressed[i] = keysPressed[i + 1];
        }
        keysPressed[maxKeys - 1] = 0;

        if (keysPressed[0] == 0) {
            return TRIGGER;
        } else {
            return NEW_NOTE;
        }
    };
    return NO_ACTION;
}

double Model::getSample(Waveform wf, double idx) {
    // we assume that idx is between 0-1
    double temp;
    int phase;
    switch (wf) {
    case TRIANGLE:
        temp = -1.0f + ((idx > 0.5f) ? 4.0f - idx * 4.0f : idx * 4.0f);
        lastSample = (lastSample * 3.0f + temp) * 0.25f;
        return lastSample * 1.5f;
    case KNEANGLE:
        temp = -0.75f + ((idx > 0.5f) ? 2.0f - idx * 2.0f : idx * 4.0f);
        lastSample = (lastSample * 3.0f + temp) * 0.25f;
        return lastSample;
    case SAWTOOTH:
        temp = idx * 2.0f - 1.0f;
        lastSample = (lastSample * 3.0f + temp) * 0.25f;
        return lastSample;
    case SQUARE:
        temp = (idx < 0.5f) ? 1.0f : -1.0f;
        lastSample = (lastSample * 3.0f + temp) * 0.25f;
        return lastSample * 0.816f;
    case SQUARE33:
        temp = (idx < 0.33f) ? 1.0f : -1.0f;
        lastSample = (lastSample * 3.0f + temp) * 0.25f;
        return lastSample * 0.816f;
    case SQUARE25:
        temp = (idx < 0.25f) ? 1.0f : -1.0f;
        lastSample = (lastSample * 3.0f + temp) * 0.25f;
        return lastSample * 0.816f;
    case TOOTHSAW:
        // osc3 only - saw backwards (for lfo)
        temp = 1.0f - idx * 2.0f;
        lastSample = (lastSample * 3.0f + temp) * 0.25f;
        return lastSample;
    }
    return 0.0f;
}

bool Model::renderNextBlock() {
    float hz, osc3mod;
    double sum;
    constexpr int chunkSize = 16;

    // glide..
    fNotePlaying = static_cast<float>(notePlaying) * (1.0f - portamentoAlpha) + (fNotePlaying * portamentoAlpha);

    // only update angle every 64 sample..
    osc3angle = AudioMath::fnoteToHz(fNotePlaying + bendSemis) * (1.0f / TPH_DSP_SR) * osc1rangeFactor;
    osc3mod = getSample(osc3wf, osc3idx); // messing up the filter in getSample though..

    osc1angle = AudioMath::fnoteToHz(fNotePlaying + bendSemis) * (1.0f / TPH_DSP_SR) * osc1rangeFactor;
    //
    osc2angle = AudioMath::fnoteToHz(fNotePlaying + bendSemis + osc2detune) * (1.0f / TPH_DSP_SR) * osc2rangeFactor;

    vcaAR.updateDelta(vcaARslope);
    if (vcaARslope.state != audio::envelope::OFF) {
        vcaEaser.setTarget(vcaARslope.currVal + vcaARslope.gap); // + lfo1.getLFOval());
        for (std::size_t i = 0; i < bufferSize; i++) {
            if (i % chunkSize == 0) {
                // osc1hz = osc1wf * osc1div;
                // osc1angle = kbd;
                // osc1hz = (osc1hz * 99.0f + hz) * 0.01f;
            }
            osc1idx += osc1angle;
            if (osc1idx > 1)
                osc1idx -= 1.0f;
            osc2idx += osc2angle;
            if (osc2idx > 1)
                osc2idx -= 1.0f;
            osc3idx += osc3angle;
            if (osc3idx > 1)
                osc3idx -= 1.0f;
            vcaEaserVal = vcaEaser.getValue(); // * lfo1vca;
            sum = 0.0f;
            sum += getSample(osc1wf, osc1idx) * osc1vol;
            sum += getSample(osc2wf, osc2idx) * osc2vol;
            sum += getSample(osc3wf, osc3idx) * osc3vol;
            sum *= 0.25f * vcaEaserVal;
            buffer[i] = static_cast<float>(sum);
        }
        vcaAR.commit(vcaARslope);
    } else {
        notePlaying = 0;
        for (std::size_t i = 0; i < bufferSize; i++) {
            buffer[i] = 0;
        }
    }

    motherboardActions();
    return false; // mono
}

void Model::motherboardActions() {
    lfo1.updatePhase();
    lfo2.updatePhase();
}

} // namespace Synth::Monolith
