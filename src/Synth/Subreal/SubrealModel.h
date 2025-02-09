#pragma once
#include "./SubrealVoice.h"
#include "Synth/SynthBase.h"
#include "Synth/SynthInterface.h"
#include "constants.h"
#include "core/audio/AudioMath.h"
#include "core/audio/envelope/ADSFR.h"
#include "core/audio/envelope/ASR.h"
#include "core/audio/filter/MultiFilter.h"
#include "core/audio/lfo/LFO.h"
#include "core/audio/misc/Easer.h"
#include "core/audio/osc/LUT.h"
#include "core/destructor/Queue.h"
#include "core/parameters/params.h"
#include <array>
#include <cstdlib>
#include <iostream>
#include <string>
#include <unordered_map>

using json = nlohmann::json;

namespace Synth::Subreal {

enum UP {
    modwheel,

    osc_mix,
    osc1_fmsens,
    osc1_detune,
    osc2_oct,
    osc2_semi,
    osc2_detune,

    osc_mix_kt,
    osc1_fmsens_kt,
    vca_pan_kt,
    osc_mix_vt,
    osc1_fmsens_vt,
    osc2_freq_kt,

    peg_atime,
    peg_rtime,
    pb_range,
    peg_asemis,
    peg_rsemis,
    noise,

    lfo1_speed,
    lfo1_depth,
    lfo1_shape,
    lfo1_routing,
    lfo1_ramp,
    lfo1_mw_control,

    vcf_cutoff,
    vcf_resonance,
    vcf_type,
    vcf_shape,
    vcf_cutoff_kt,
    vcf_cutoff_vt,

    vcf_attack,
    vcf_decay,
    vcf_sustain,
    vcf_release,
    vcf_fade,
    vcf_rate_kt,

    vca_attack,
    vca_decay,
    vca_sustain,
    vca_release,
    vca_fade,
    vca_rate_kt,

    lfo2_speed,
    lfo2_depth,
    lfo2_shape,
    lfo2_routing,
    not_used_1,
    lfo2_mw_control,

    up_count
};

namespace LFO1 {
enum Routing {
    off,
    osc12,
    osc1,
    osc2,
    //    vcf,
    //    vca,
    _count
};
}

namespace LFO2 {
enum Routing {
    off,
    fmSens,
    vcf,
    vca,
    _count
};
}

namespace MW {

enum LFOcontrol {
    off,
    depth,
    speed,
    _count
};
} // namespace MW

// NOTE: These are currently not used - let's skip them..
/*
namespace OSC1 {
enum WF {
    sine,
    tri,
    square,
    saw,
    noise,
    lut1,
    lut2,
    _count
};
} // namespace OSC1

namespace OSC2 {
enum class WF {
    sine,
    tri,
    square,
    saw,
    noise,
    lut1,
    lut2,
    _count
};
}
*/

const std::array<int, 11> pegDepth2Semis = {-12, -7, -5, -2, -1, 0, 1, 2, 5, 7, 12};

class Model : public SynthBase {

  public:
    // Constructor
    Model();

    // Destructor - delete luts.
    ~Model();

    // Public methods. These should match interface right (contract)
    void reset() override;
    void initSettings();
    // void updateSetting(std::string key, std::string value);
    // void updateSetting(const std::string &key, const std::string &value, void *object, uint32_t size, bool isStereo);
    void updateSetting(const std::string &type, void *object, uint32_t size, bool isStereo, Destructor::Record &recordDelete) override; //, Constructor::Queue &constructorQueue) override;

    void bindBuffers(float *audioBufferLeft, float *audioBufferRight, std::size_t bufferSize);

    json getParamDefsAsJSON() override {
        return SynthBase::getParamDefsAsJson();
    }

    void pushStrParam(const char *name, float val) override {
        return SynthBase::pushStrParam(name, val);
    }

    // Method to parse MIDI commands
    void parseMidi(uint8_t cmd, uint8_t param1, uint8_t param2) override;

    void updateVoiceLUT(const audio::osc::LUT &lut, int no);

    // Method to render the next block of audio
    bool renderNextBlock() override;

    void addToLeftSample(std::size_t sampleIdx, float val);
    void addToRightSample(std::size_t sampleIdx, float val);

    const audio::osc::LUT &getLUT1() const;
    const audio::osc::LUT &getLUT2() const;

    uint8_t masterTune; // copy from audioHallway

    // params here.
    float modwheel = 0.0f;
    float osc_mix = 0.5f;
    float osc1_fmsens = 0.0f;
    float osc1_detune = 0.0f;
    float osc_mix_kt = 0.0f;
    float osc1_fmsens_kt = 0.0f;
    float osc1_fmsens_vt = 0.0f;

    int osc2_oct = 0;
    int osc2_semi = 0;
    float osc2_detune = 0.0f;
    float osc2_freq_kt = 1;
    float osc_mix_vt = 0;

    float vcf_cutoff = 500;
    float vcf_resonance = 0.5;
    float vcf_cutoff_kt = 0.0f;
    float vcf_cutoff_vt = 0.0f;
    float vcf_rate_kt = 0;
    float vca_rate_kt = 0;

    float lfo1_depth = 0.5;
    float lfo1_ramp = 0.0f;
    float lfo2_depth = 0.5;
    LFO1::Routing lfo1_routing;
    LFO2::Routing lfo2_routing;
    float vca_pan_kt;

    int peg_asemis;
    int peg_rsemis;
    float noise_o2 = 0;
    float noise_vcf = 0;

    int pb_range;
    MW::LFOcontrol lfo1_mw_control;
    MW::LFOcontrol lfo2_mw_control;

    // more complex stuff here. May be modified by params..
    float lfo1_speed = 0;
    float lfo2_speed = 0;
    float bendCents = 0;
    float senseTracking = 0.0f;
    audio::envelope::ADSFR vcaAR;
    audio::envelope::ADSFR vcfAR;
    audio::envelope::ASR pegAR;
    audio::lfo::Standard lfo1; // change to ramp. duh. no, it's in voice..
    audio::lfo::Standard lfo2;
    audio::filter::FilterType filterType = audio::filter::FilterType::highPass;
    audio::filter::FilterPoles filterPoles = audio::filter::FilterPoles::p2;

    // private stuff that user should mess with.
    float *bufferLeft, *bufferRight; // Pointer to audio buffer, minimize write so:
    float synthBufferLeft[TPH_RACK_RENDER_SIZE], synthBufferRight[TPH_RACK_RENDER_SIZE];
    audio::osc::LUT *lut1 = nullptr;
    audio::osc::LUT *lut2 = nullptr;
    std::size_t bufferSize; // Size of the audio buffer
    bool vcfInverse = false;
    int debugCount = 0;

  private:
    void buildLUT(audio::osc::LUT &lut, const std::string val);

    void setLFO1speed();

    void setLFO2speed();

  private:
  protected:
    std::vector<Voice> voices; // Vector to hold Voice objects

    int8_t findVoiceToAllocate(uint8_t note);

    void motherboardActions();

    void initializeParameters();
    //
    void setupParams(int upCount);
};

} // namespace Synth::Subreal
