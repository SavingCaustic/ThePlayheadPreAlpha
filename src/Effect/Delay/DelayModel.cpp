#include "./DelayModel.h"
#include <vector>

namespace Effect::Delay {

// Constructor to accept buffer and size
Model::Model() {
    // ok now we lock buffersize assuming 48k. 48k * 1.28 => 128kB
    reset();
    setupParams(UP::up_count); // creates the array with attributes and lambdas for parameters - NOT INTERFACE
    EffectBase::initParams();
    delayBuffer.reserve(BUFFER_SIZE);
    delayBuffer.resize(BUFFER_SIZE); // maybe later times 2..
    std::fill(delayBuffer.begin(), delayBuffer.end(), 0.0f);
}

void Model::reset() {
    wrPointer = 0;
    rdPointer = 0;
}

void Model::setupParams(int upCount) {
    if (EffectBase::paramDefs.empty()) {
        // after declaration, indexation requested, see below..
        EffectBase::paramDefs = {
            {UP::time, {"time", 0.5f, 0, true, 20, 6, [this](float v) {
                            // 20 - 1280 mS
                            time = v / 1000;
                            sampleGap = round(time * TPH_DSP_SR);
                            std::cout << "time:" << time << std::endl;
                        }}},
            {UP::mix, {"mix", 0.5f, 0, false, 0, 1, [this](float v) {
                           mix = v;
                       }}},
            {UP::feedback, {"feedback", 0.5f, 0, false, 0, 1, [this](float v) {
                                std::cout << "setting feedback.." << std::endl;
                                feedback = v;
                            }}},
            {UP::highcut, {"highcut", 0.0f, 0, false, 0, 1, [this](float v) {
                               std::cout << "highcut " << v << std::endl;
                               highcut = v;
                           }}},
            {UP::noise, {"noise", 0.0f, 0, false, 0, 1, [this](float v) {
                             noise = v;
                         }}}}; // Removed the extra semicolon here
        EffectBase::indexParams(upCount);
    }
}

bool Model::renderNextBlock(bool isSterero) {
    // agnostic. Stereo-processing but keeping monoOut if monoIn
    debugCnt++;
    if (debugCnt % 1024 == 0) {
        // std::cout << "debug: " << bufferSize << std::endl;
    }

    float delayOutL, delayOutR;
    float audioInL, audioInR;
    int currGap = 0.5f * ((wrPointer - rdPointer) & (BUFFER_SIZE - 1));
    AudioMath::easeLog5(sampleGap, sampleGapEaseOut);
    rdPointer = (wrPointer - static_cast<int>(round(sampleGapEaseOut) * 2)) & (BUFFER_SIZE - 1);

    for (std::size_t i = 0; i < bufferSize; i += 2) {
        //
        delayOutL = delayBuffer[rdPointer];
        delayOutR = delayBuffer[rdPointer + 1];
        // calc rms
        RMS = RMS * 0.95f + (delayOutL + delayOutR) * (delayOutL + delayOutR) * 0.05f;
        //
        noisePink = noisePink * 0.8f + AudioMath::noise() * 0.2f; // 2 kHz
        highCutLeft = highCutLeft * 0.9f + delayOutL * 0.1f;      // about 1 khz
        highCutRight = highCutRight * 0.9f + delayOutR * 0.1f;
        delayOutL = (highCutLeft * highcut + delayOutL * (1.0f - highcut));
        delayOutR = (highCutRight * highcut + delayOutR * (1.0f - highcut));
        // add noise
        delayOutL += RMS * noisePink * noise * 0.2f;
        delayOutR += RMS * noisePink * noise * 0.2f;
        //
        audioInL = buffer[i];
        audioInR = buffer[i + 1];
        //
        delayBuffer[wrPointer] = audioInL * (1 - feedback) + delayOutL * feedback;
        delayBuffer[wrPointer + 1] = audioInR * (1 - feedback) + delayOutR * feedback;
        //
        buffer[i] = delayOutL * mix + audioInL * (1 - mix);
        buffer[i + 1] = delayOutR * mix + audioInR * (1 - mix);
        //
        wrPointer = (wrPointer + 2) & (BUFFER_SIZE - 1);
        rdPointer = (rdPointer + 2) & (BUFFER_SIZE - 1);
    }
    return isSterero;
}

} // namespace Effect::Delay
