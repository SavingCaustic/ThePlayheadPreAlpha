#include "./SubrealVoice.h"
#include "./SubrealModel.h"

namespace Synth::Subreal {

Voice::Voice(Model &model)
    : modelRef(model),
      osc1(), // Initialize osc1 with LUT from model
      osc2()  // Initialize osc2 with LUT from model
{
    reset();
}

void Voice::reset() {
    // Called on setup
    notePlaying = 255; // unsigned byte. dunno..
}

void Voice::setLUTs(const audio::osc::LUT &lut, int no) {
    if (no == 1) {
        osc1.setLUT(lut); // Set LUT for osc1
    } else {
        osc2.setLUT(lut); // Set LUT for osc2
    }
}

void Voice::noteOn(uint8_t midiNote, float velocity) {
    notePlaying = midiNote;
    noteVelocity = velocity;
    mixAmplitude = noteVelocity * 0.4f; // Factor to avoid too much dist on polyphony..
    lfo1_ramp_avg = 0.0f;               // even if retrigger??
    //
    osc_mix_kv = ((midiNote * 0.02f - 1) * modelRef.osc_mix_kt + 1.0f) *
                 ((velocity * 2.0f - 1) * modelRef.osc_mix_vt + 1.0f);

    osc1_fmsens_kv = ((midiNote * 0.02f - 1) * modelRef.osc1_fmsens_kt + 1.0f) *
                     ((velocity * 2.0f - 1) * modelRef.osc1_fmsens_kt + 1.0f);

    leftAtt = fmin(1, fmax(0, (notePlaying - 60) * 0.04f * modelRef.vca_pan_kt));
    rightAtt = fmin(1, fmax(0, (60 - notePlaying) * 0.04f * modelRef.vca_pan_kt));
    tracking = fmax(0, (2.0f + modelRef.senseTracking * AudioMath::noteToFloat(notePlaying) * 7));
    //
    modelRef.vcaAR.triggerSlope(vcaARslope, audio::envelope::ADSFRCmd::NOTE_ON);
    modelRef.vcfAR.triggerSlope(vcfARslope, audio::envelope::ADSFRCmd::NOTE_ON);
    modelRef.pegAR.triggerSlope(pegARslope, audio::envelope::ASRCmd::NOTE_ON);
    // peg influence = (1 - pegAr.level) * semis
    filter.initFilter();
}

void Voice::noteOff() {
    // enter release state in all envelopes.
    modelRef.vcaAR.triggerSlope(vcaARslope, audio::envelope::ADSFRCmd::NOTE_OFF);
    modelRef.vcfAR.triggerSlope(vcfARslope, audio::envelope::ADSFRCmd::NOTE_OFF);
    modelRef.pegAR.triggerSlope(pegARslope, audio::envelope::ASRCmd::NOTE_OFF);
}

audio::envelope::ADSFRState Voice::getVCAstate() {
    return vcaARslope.state;
}

float Voice::getVCAlevel() {
    return vcaARslope.currVal;
}

bool Voice::checkVoiceActive() {
    return vcaARslope.state != audio::envelope::ADSFRState::OFF;
}

bool Voice::renderNextVoiceBlock(std::size_t bufferSize) {
    float osc1hz, osc2hz, lfoCents;
    // Chunk could be set to SIMD-capability.
    constexpr int chunkSize = 16;
    float chunkSample[chunkSize]; // cache-optimized storage for chunks. (mono)
    if (vcaARslope.state != audio::envelope::ADSFRState::OFF) {
        // calc lfo1 ramp
        lfo1_ramp_avg = modelRef.lfo1_depth * modelRef.lfo1_ramp + lfo1_ramp_avg * (1 - modelRef.lfo1_ramp);
        float lfo1cents = modelRef.lfo1.getLFOval() * lfo1_ramp_avg * 200.0f;
        if (modelRef.lfo1_mw_control == MW::LFOcontrol::depth) {
            lfo1cents *= modelRef.modwheel;
        }
        // calc lfo2
        float lfo2amp = modelRef.lfo2_depth;
        if (modelRef.lfo2_mw_control == MW::LFOcontrol::depth) {
            lfo2amp *= modelRef.modwheel;
        }

        // PEG
        float pegCents = 0;
        if (pegARslope.state == audio::envelope::ASRState::ATTACK) {
            pegCents = (1 - pegARslope.currVal) * modelRef.peg_asemis * 100;
        }
        if (pegARslope.state == audio::envelope::ASRState::RELEASE) {
            pegCents = (1 - pegARslope.currVal) * modelRef.peg_rsemis * 100;
        }

        // calc osc2
        float osc2noteFloat = 60 + (notePlaying - 60) * modelRef.osc2_freq_kt + modelRef.osc2_semi + modelRef.osc2_oct * 12;
        // for cents, start with lfo so we can diminish with mw.
        float osc2cents = modelRef.bendCents + modelRef.osc2_detune + pegCents;
        if (modelRef.lfo1_routing == LFO1::Routing::osc12 || modelRef.lfo1_routing == LFO1::Routing::osc2) {
            osc2cents += lfo1cents;
        }
        // done here? we should add the PEG ASR..

        // osc2hz = AudioMath::noteToHz(osc2note, osc2cents);
        osc2hz = AudioMath::fnoteToHz(osc2noteFloat + osc2cents * 0.01f);
        osc2.setAngle(osc2hz);
        // done with osc2.

        // calc fmSens driving osc1.
        float fmAmp = modelRef.osc1_fmsens * osc1_fmsens_kv;
        if (modelRef.lfo2_routing == LFO2::Routing::fmSens) {
            fmAmp *= (1 + modelRef.lfo2.getLFOval() * lfo2amp);
        }

        // calc note and cent for osc1
        float osc1cents = modelRef.bendCents + modelRef.osc1_detune + pegCents;
        if (modelRef.lfo1_routing == LFO1::Routing::osc12 || modelRef.lfo1_routing == LFO1::Routing::osc1) {
            osc1cents += lfo1cents;
        }

        osc1hz = AudioMath::noteToHz(notePlaying, osc1cents);
        osc1.setAngle(osc1hz);

        // setup new delta (lin-easer) for VCA
        modelRef.vcaAR.updateDelta(vcaARslope);
        float vcaTarget = vcaARslope.currVal + vcaARslope.gap;
        // same for VCF
        modelRef.vcfAR.updateDelta(vcfARslope);
        float vcfTarget = vcfARslope.currVal + vcfARslope.gap;

        modelRef.pegAR.updateDelta(pegARslope);

        if (modelRef.lfo1_routing == LFO1::Routing::vca) {
            vcaTarget *= (modelRef.lfo1.getLFOval() * modelRef.lfo1_depth) + 1.0f;
        }
        if (modelRef.lfo2_routing == LFO2::Routing::vca) {
            vcaTarget *= (modelRef.lfo2.getLFOval() * lfo2amp) + 1.0f;
        }
        vcaEaser.setTarget(vcaTarget);
        vcfEaser.setTarget(vcfTarget);

        for (std::size_t i = 0; i < bufferSize; i += chunkSize * 2) {
            // chunk here.. 1/2/3. Oscillators | Filter | VCA
            AudioMath::easeLog50(fmAmp, fmAmpEaseOut);
            AudioMath::easeLog5(modelRef.osc_mix * osc_mix_kv, oscMixEaseOut);
            for (std::size_t j = 0; j < chunkSize; j++) {
                chunkSample[j] = osc2.getNextSample(0);
            }
            /*
            if (modelRef.osc2noiseMix > 0.02f) {
                for (std::size_t j = 0; j < chunkSize; j++) {
                    float noise = AudioMath::noise();
                    chunkSample[j] = chunkSample[j] * (1.0f - modelRef.osc2noiseMix) + noise * modelRef.osc2noiseMix;
                }
            }
            */
            for (std::size_t j = 0; j < chunkSize; j++) {
                float y1 = osc1.getNextSample(chunkSample[j] * fmAmpEaseOut);
                chunkSample[j] = y1 * (1 - oscMixEaseOut) + chunkSample[j] * oscMixEaseOut;
            }
            // VCF (100Hz is appropirate for lpf - maybe not for others..)
            vcfEaserVal = vcfEaser.getValue();
            if (modelRef.vcfInverse) {
                filter.setCutoff(100 + modelRef.vcf_cutoff * (1.0f - vcfEaserVal));
            } else {
                filter.setCutoff(100 + modelRef.vcf_cutoff * vcfEaserVal);
            }
            filter.setResonance(modelRef.vcf_resonance);
            filter.setPoles(modelRef.filterPoles);
            filter.setFilterType(modelRef.filterType);
            filter.initFilter(); //??
            // disabled while debugging:
            filter.processBlock(chunkSample, chunkSize); //, vcfEaserVal);
            // VCA
            for (std::size_t j = 0; j < chunkSize; j++) {
                vcaEaserVal = vcaEaser.getValue();
                chunkSample[j] = chunkSample[j] * vcaEaserVal * mixAmplitude;
            }
            // send to model (sum)
            for (std::size_t j = 0; j < chunkSize; j++) {
                modelRef.addToSample(i + j * 2, chunkSample[j] * (1 - leftAtt));
                modelRef.addToSample(i + j * 2 + 1, chunkSample[j] * (1 - rightAtt));
            }
        }

        // copy local voice-buffer to synth-buffer
        modelRef.vcaAR.commit(vcaARslope);
        modelRef.vcfAR.commit(vcfARslope);
        modelRef.pegAR.commit(pegARslope);
    } else {
        // Remove voice from playing
        // maybe use return-type
    }
    return true;
}

} // namespace Synth::Subreal