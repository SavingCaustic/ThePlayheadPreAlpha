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
    // HEY! noteVelocity or mixAmplitude must ease!
    lfo1_ramp_avg = 0.0f; // even if retrigger??
    //
    osc_mix_kv = ((midiNote * 0.02f - 1) * modelRef.osc_mix_kt + 1.0f) *
                 ((velocity * 2.0f - 1) * modelRef.osc_mix_vt + 1.0f);

    osc1_fmsens_kv = ((midiNote * 0.02f - 1) * modelRef.osc1_fmsens_kt + 1.0f) *
                     ((velocity * 2.0f - 1) * modelRef.osc1_fmsens_vt + 1.0f);

    vcf_cutoff_kv = ((midiNote * 0.02f - 1) * modelRef.vcf_cutoff_kt + 1.0f) *
                    ((velocity * 2.0f - 1) * modelRef.vcf_cutoff_vt + 1.0f);

    leftAtt = fmin(1, fmax(0, (notePlaying - 60) * 0.04f * modelRef.vca_pan_kt));
    rightAtt = fmin(1, fmax(0, (60 - notePlaying) * 0.04f * modelRef.vca_pan_kt));
    tracking = fmax(0, (2.0f + modelRef.senseTracking * AudioMath::noteToFloat(notePlaying) * 7));
    //
    // vca-tracking
    vcaARslope.kGain = exp2((notePlaying - 64) / 64.0f * modelRef.vca_rate_kt);
    FORMAT_LOG_MESSAGE(modelRef.logTemp, LOG_INFO, "setting VCA kGain to %f", vcaARslope.kGain);
    modelRef.sendAudioLog();

    modelRef.vcaAR.triggerSlope(vcaARslope, audio::envelope::ADSFRCmd::NOTE_ON);
    // vcf-tracking
    vcfARslope.kGain = exp2((notePlaying - 64) / 64.0f * modelRef.vcf_rate_kt);
    modelRef.vcfAR.triggerSlope(vcfARslope, audio::envelope::ADSFRCmd::NOTE_ON);

    pegARslope.kGain = 1.0f;
    modelRef.pegAR.triggerSlope(pegARslope, audio::envelope::ASRCmd::NOTE_ON);
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
    float osc1hz, osc2hz, lfoCents, lfo1cents, lfo2amp;

    if (vcaARslope.state != audio::envelope::ADSFRState::OFF) {
        // :: LFO1 ::
        // calc ramp
        lfo1_ramp_avg = modelRef.lfo1_depth * modelRef.lfo1_ramp + lfo1_ramp_avg * (1 - modelRef.lfo1_ramp);
        float lfo1cents = modelRef.lfo1.getLFOval() * lfo1_ramp_avg * 1200.0f;
        if (modelRef.lfo1_mw_control == MW::LFOcontrol::depth) {
            lfo1cents *= modelRef.modwheel;
        }
        // :: LFO2 ::
        // calc lfo2
        lfo2amp = modelRef.lfo2_depth;
        if (modelRef.lfo2_mw_control == MW::LFOcontrol::depth) {
            lfo2amp *= modelRef.modwheel;
        }

        // :: PEG ::
        float pegCents = 0;
        switch (pegARslope.state) {
        case audio::envelope::ASRState::ATTACK:
            pegCents = (1 - pegARslope.currVal) * modelRef.peg_asemis * 100;
            break;
        case audio::envelope::ASRState::RELEASE:
            pegCents = (1 - pegARslope.currVal) * modelRef.peg_rsemis * 100;
            break;
        case audio::envelope::ASRState::OFF:
            // clinging vca-release..
            pegCents = modelRef.peg_rsemis * 100;
            break;
        }

        // :: OSC2 ::
        float osc2noteFloat = 60 + (notePlaying - 60) * modelRef.osc2_freq_kt + modelRef.osc2_semi + modelRef.osc2_oct * 12;
        // for cents, start with lfo so we can diminish with mw.
        float osc2cents = modelRef.bendCents + modelRef.osc2_detune + pegCents;
        if (modelRef.lfo1_routing == LFO1::Routing::osc12 || modelRef.lfo1_routing == LFO1::Routing::osc2) {
            osc2cents += lfo1cents;
        }

        // osc2hz = AudioMath::noteToHz(osc2note, osc2cents);
        osc2hz = AudioMath::fnoteToHz(osc2noteFloat + osc2cents * 0.01f);
        osc2.setAngle(osc2hz);
        // done with osc2.

        // :: OSC1 ::
        float fmAmp = modelRef.osc1_fmsens * osc1_fmsens_kv;
        if (modelRef.lfo2_routing == LFO2::Routing::fmSens) {
            // fmAmp *= (1 + modelRef.lfo2.getLFOval()) * lfo2amp * 5.0f;
            fmAmp += modelRef.lfo2.getLFOval() * lfo2amp * 5.0f;
        }
        if (fmAmp < 0)
            fmAmp = 0;

        // calc note and cent for osc1
        float osc1cents = modelRef.bendCents + modelRef.osc1_detune + pegCents;
        if (modelRef.lfo1_routing == LFO1::Routing::osc12 || modelRef.lfo1_routing == LFO1::Routing::osc1) {
            osc1cents += lfo1cents;
        }
        osc1hz = AudioMath::noteToHz(notePlaying, osc1cents);
        osc1.setAngle(osc1hz);

        // LFO1 reworked to be only pitch so comment out..
        /*
        if (modelRef.lfo1_routing == LFO1::Routing::vca) {
            // vcaTarget *= 1.0f + (modelRef.lfo1.getLFOval() * modelRef.lfo1_depth) - modelRef.lfo1_depth;
            // modTarget = modTarget - modTarget * modelRef.lfo1_depth + (modelRef.lfo1.getLFOval() + 1.0f) * modTarget * modelRef.lfo1_depth * 0.5f;
        }
        */
        mixAmplitude = noteVelocity * 0.4f; // Factor to avoid too much dist on polyphony..
        if (modelRef.lfo2_routing == LFO2::Routing::vca) {
            mixAmplitude *= 1.0f + (modelRef.lfo2.getLFOval() * lfo2amp) - lfo2amp;
        }
        for (std::size_t i = 0; i < bufferSize; i += chunkSize) {
            // chunk here.. 1/2/3. Oscillators | Filter | VCA
            AudioMath::easeLog50(fmAmp, fmAmpEaseOut);
            AudioMath::easeLog5(modelRef.osc_mix * osc_mix_kv, oscMixEaseOut);
            for (std::size_t j = 0; j < chunkSize; j++) {
                chunkSample[j] = osc2.getNextSample(0);
            }
            for (std::size_t j = 0; j < chunkSize; j++) {
                float y1 = osc1.getNextSample(chunkSample[j] * fmAmpEaseOut);
                chunkSample[j] = y1 * (1 - oscMixEaseOut) + chunkSample[j] * oscMixEaseOut;
            }
            // VCF (100Hz is appropirate for lpf - maybe not for others..)
            if (modelRef.filterType != audio::filter::FilterType::bypass) {
                float vcfVal;
                if (modelRef.vcfInverse) {
                    vcfVal = (1.0f - vcfARslope.currVal);
                } else {
                    vcfVal = vcfARslope.currVal;
                }
                vcfEaserVal = vcfEaserVal * 0.9 + vcfVal * 0.1;
                filter.setCutoff(100 + modelRef.vcf_cutoff * vcfEaserVal * vcf_cutoff_kv);
                filter.setResonance(modelRef.vcf_resonance);
                filter.setPoles(modelRef.filterPoles);
                filter.setFilterType(modelRef.filterType);
                filter.initFilter(); //??
                // akward way to fast forward..
                vcfARslope.currVal = vcfARslope.currVal * (1.0f - vcfARslope.k * 16.0f) + vcfARslope.targetVal * vcfARslope.k * 16.0f;
                filter.processBlock(chunkSample, chunkSize); //, vcfEaserVal);
            }
            if (vcaARslope.targetVal > 2) {
                // clamp on excessive attack (kt)
                vcaARslope.targetVal = 2;
            }
            float delta = vcaARslope.k * (vcaARslope.targetVal - vcaARslope.currVal);
            for (std::size_t j = 0; j < chunkSize; j++) {
                mixAmpAvg = mixAmpAvg * 0.95f + mixAmplitude * 0.05f;
                vcaARslope.currVal += delta;
                chunkSample[j] = chunkSample[j] * vcaARslope.currVal * mixAmpAvg;
            }
            // send to model (sum)
            for (std::size_t j = 0; j < chunkSize; j++) {
                modelRef.addToLeftSample(i + j, chunkSample[j] * (1 - leftAtt));
                modelRef.addToRightSample(i + j, chunkSample[j] * (1 - rightAtt));
            }
        }
        // commit easers (this way to avoid float rounding errors)
        modelRef.vcaAR.updateDelta(vcaARslope);
        modelRef.vcfAR.updateDelta(vcfARslope);

        pegARslope.currVal = pegARslope.currVal * (1.0f - pegARslope.k * 64.0f) + pegARslope.targetVal * pegARslope.k * 64.0f;
        modelRef.pegAR.updateDelta(pegARslope);

    } else {
        // Remove voice from playing??
        // maybe use return-type
    }
    return true;
}

} // namespace Synth::Subreal