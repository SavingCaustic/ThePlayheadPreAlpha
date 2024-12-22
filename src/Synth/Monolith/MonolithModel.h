#pragma once

#include "Synth/SynthBase.h"
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

using json = nlohmann::json;

namespace Synth::Monolith {
constexpr int maxKeys = 8; // Maximum number of keys that can be stored in the array to resolve on key-release

enum UP {
    // think again about the router.. we want the scroller position to be synth-aware, right..
    // maybe an object makes more sense than a class. Easier to reference..
    kbd_glide,
    osc1_detune,
    osc1_range,
    osc1_wf,
    osc1_vol,
    osc2_detune,
    osc2_range,
    osc2_wf,
    osc2_vol,
    osc3_detune,
    osc3_range,
    osc3_wf,
    osc3_vol,
    osc3_tracking,
    vcf_cutoff,
    vcf_resonance,
    vcf_type,
    vcf_shape,
    vca_attack,
    vca_decay,
    vca_sustain,
    vca_fade,
    vca_release,
    lfo1_speed,
    up_count
};

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
    NUM_WAVEFORMS,
    TOOTHSAW // special for osc3 - replacing kneangle
};

class Model : public SynthBase {
  public:
    // Constructor
    Model();

    // Public methods. These should match interface right (contract)
    void reset() override;
    void setupParams(int upCount);

    static std::unordered_map<int, ParamDefinition> SparamDefs;
    static std::unordered_map<std::string, int> SparamIndex;

    json getParamDefsAsJSON() override {
        return SynthBase::getParamDefsAsJson();
    }

    void pushStrParam(const std::string &name, float val) override {
        return SynthBase::pushStrParam(name, val);
    }

    // Method to parse MIDI commands
    void parseMidi(uint8_t cmd, uint8_t param1, uint8_t param2) override;

    // Method to render the next block of audio
    bool renderNextBlock() override;

    double getSample(Waveform wf, double idx);
    KeyAction pressKey(u_int8_t key);
    KeyAction releaseKey(u_int8_t key);

    Waveform osc1type;
    Waveform osc2type;

    float filterCutoff = 500;
    float filterResonance = 0.5;
    bool vcfInverse = false;
    audio::filter::FilterType filterType = audio::filter::FilterType::highPass;
    audio::filter::FilterPoles filterPoles = audio::filter::FilterPoles::p2;

  protected:
    audio::filter::MultiFilter filter;
    audio::envelope::ADSFR vcaAR;
    audio::envelope::ADSFRSlope vcaARslope;
    audio::lfo::Standard lfo1;
    audio::lfo::Standard lfo2;
    audio::misc::Easer oscMixEaser;
    audio::misc::Easer vcaEaser;
    float velocityLast = 0; // super-easy easer
    float vcaEaserVal;
    Waveform osc1wf = TRIANGLE;
    Waveform osc2wf = SQUARE;
    Waveform osc3wf = SAWTOOTH;
    float lastSample = 0;
    u_int8_t keysPressed[8] = {0}; // 0 = no note

    // void initFilter();

    // void applyFilter(float &sample);

    // void applySine(float multiple, float amplitude);

    // void renderVoice();

    void motherboardActions();

    // void initializeParameters();
    //  Handle incoming MIDI CC messages
    // void handleMidiCC(uint8_t ccNumber, float value);
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
    int osc1rangeFactor = 1;
    float osc1detune = 0.0f;
    float osc1vol = 0.5f;
    // osc2
    double osc2angle;
    double osc2idx;
    float osc2hz = 440.0f;
    float osc2detune = 0.0f;
    int osc2rangeFactor = 1;
    float osc2vol = 0.5f;
    // osc2
    double osc3angle;
    double osc3idx;
    float osc3hz = 440.0f;
    float osc3detune = 0.0f;
    int osc3rangeFactor = 1;
    float osc3vol = 0.5f;
    float osc3tracking = 1.0f;
};

} // namespace Synth::Monolith
