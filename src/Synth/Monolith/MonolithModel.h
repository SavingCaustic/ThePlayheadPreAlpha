#pragma once

#include "Synth/SynthInterface.h"
#include "constants.h"
#include "core/audio/AudioMath.h"
#include "core/audio/envelope/ADSFR.h"
#include "core/audio/filter/MultiFilter.h"
#include "core/audio/lfo/LFO.h"
#include "core/audio/misc/Easer.h"
#include "core/parameters/params.h"
#include <array>
#include <cstdlib>
#include <iostream>
#include <string>
#include <unordered_map>

namespace Synth::Monolith {
constexpr int maxKeys = 8; // Maximum number of keys that can be stored in the array to resolve on key-release

enum KeyAction {
    NO_ACTION,
    NEW_NOTE,
    TRIGGER
};

enum Waveform {
    TRIANGLE,
    KNEANGLE,
    SAWTOOTH,
    SQUARE,
    SQUARE33,
    SQUARE25,
    NUM_WAVEFORMS
};

enum Waveform3 {
    TRIANGLE3,
    SAWTOOTH3,
    SQUARE3,
    SQUARE333,
    SQUARE253,
    NUM_WAVEFORMS3
};

class Model : public SynthInterface {
  public:
    // Constructor
    Model(float *audioBuffer, std::size_t bufferSize);
    // Public methods. These should match interface right (contract)
    void reset() override;

    // Method to parse MIDI commands
    void parseMidi(uint8_t cmd, uint8_t param1, uint8_t param2) override;
    bool pushMyParam(const std::string &name, float val);

    // Method to render the next block of audio
    bool renderNextBlock() override;

    float getSample(Waveform wf, double idx);
    KeyAction pressKey(u_int8_t key);
    KeyAction releaseKey(u_int8_t key);

    Waveform osc1type;
    Waveform osc2type;

  protected:
    float *buffer;          // Pointer to audio buffer
    std::size_t bufferSize; // Size of the audio buffer

    audio::filter::MultiFilter filter;
    audio::envelope::ADSFR vcaAR;
    audio::envelope::Slope vcaARslope;
    audio::lfo::RampLfo lfo1;
    audio::lfo::SimpleLfo lfo2;
    audio::misc::Easer oscMixEaser;
    audio::misc::Easer vcaEaser;
    float velocityLast = 0; // super-easy easer
    float vcaEaserVal;
    Waveform osc1wf = TRIANGLE;
    Waveform osc2wf = SQUARE;
    float lastSample = 0;
    u_int8_t keysPressed[8] = {0}; // 0 = no note

    void initFilter();

    void applyFilter(float &sample);

    void applySine(float multiple, float amplitude);

    // void renderVoice();

    void motherboardActions();

    void initializeParameters();
    // Handle incoming MIDI CC messages
    void handleMidiCC(uint8_t ccNumber, float value);
    //
    std::unordered_map<Waveform, std::vector<float>> lutTables[LUT_SIZE];

    void setupParams();
    int debugCount = 0;
    float lfo1Depth = 0.5;
    float fmSens = 0.0f;
    float senseTracking = 0.0f;
    float lfo1vca = 0.0f;

  private:
    // keyboard
    int notePlaying = 0;    // 0 = no note
    float fNotePlaying = 0; // note generating hz. portamento support.
    float portamentoAlpha = 0;
    float noteVelocity = 0; // 0-1;
    float bendSemis = 0;
    // osc1
    double osc1angle;
    double osc1idx;
    float osc1hz = 440.f;
    float osc1rangeFactor = 1.0f;
    float osc1detune = 0.0f;
    // osc2
    double osc2angle;
    double osc2idx;
    float osc2hz = 440.0f;
    float osc2detune = 0.0f;
    float osc2rangeFactor = 1.0f;
};

} // namespace Synth::Monolith
