#include "./DelayModel.h"
#include <vector>

namespace Effect::Delay {

Model::Model() {
    reset();
    setupParams(UP::up_count); // creates the array with attributes and lambdas for parameters - NOT INTERFACE
    EffectBase::initParams();
    delayBufferLeft.reserve(BUFFER_SIZE);
    delayBufferLeft.resize(BUFFER_SIZE);
    std::fill(delayBufferLeft.begin(), delayBufferLeft.end(), 0.0f);
    delayBufferRight.reserve(BUFFER_SIZE);
    delayBufferRight.resize(BUFFER_SIZE);
    std::fill(delayBufferRight.begin(), delayBufferRight.end(), 0.0f);
}

void Model::reset() {
    wrPointer = 8192;
    rdPointer = 0;
}

void Model::setupParams(int upCount) {
    if (EffectBase::paramDefs.empty()) {
        // after declaration, indexation requested, see below..
        EffectBase::paramDefs = {
            {UP::time, {"time", 0.5f, 11, false, 0, 10, [this](float v) {
                            delayInClocks = idx2clocks[static_cast<uint8_t>(v)];
                            // trust the clock to set the actual buffer index.
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
                         }}}};
        EffectBase::indexParams(upCount);
    }
}

void Model::processClock(const uint8_t clock24) {
    if (clock24 % 12 == 0) {
        clockTick = true;
    }

    // for every even 8th, measure the distance in sampleTime compared to where we were last 8th
    if (clock24 % 12 == 0) {
        // Calculate the gap between the current write pointer and the last 8th note write position
        int testSampleGap = (wrPointer + 65536 - this->last8thWritePos) % 65536;

        this->last8thWritePos = wrPointer; // Update the position of the last 8th note
        // If the gap is significantly different from the last known gap, update the tempo
        if (abs(this->clockSampleGap * 12 - testSampleGap) > 100) {
            this->clockSampleGap = testSampleGap / 12;
            FORMAT_LOG_MESSAGE(logTemp, LOG_INFO,
                               "Tempo change detected.. gap now %d", this->clockSampleGap);
            audioHallway.logMessage(logTemp);
        }
    }
}

bool Model::renderNextBlock(bool isStereo) {
    // Always process left..
    uint16_t rdPtr = rdPointer;
    uint16_t wrPtr = wrPointer;
    debugCnt++;
    // 1) Read from delay buffer
    for (uint8_t i = 0; i < bufferSize; i++) {
        iBuffer[i] = delayBufferLeft[rdPtr++];
        highCutLeft = highCutLeft * 0.9f + iBuffer[i] * 0.1f; // about 1 khz
        iBuffer[i] = (highCutLeft * highcut + iBuffer[i] * (1.0f - highcut));
    }

    // 2) Mix with input and write back to delay buffer
    for (uint8_t i = 0; i < bufferSize; i++) {
        iBuffer[i] = bufferLeft[i] * (1 - feedback) + iBuffer[i] * feedback;
        delayBufferLeft[wrPtr++] = iBuffer[i];
        //}

        // 3) Mix output and write back to host buffer
        // for (uint16_t i = 0; i < bufferSize; i++) {
        bufferLeft[i] = bufferLeft[i] * (1 - mix) + iBuffer[i] * mix;
        // just avoid noise now..
        bufferRight[i] = bufferLeft[i];
    }
    int iTemp = wrPointer; // we must use position *before its updated.
    wrPointer = wrPtr;
    iTemp -= clockSampleGap * delayInClocks;
    rdPointer = (iTemp + 65536) % 65536;

    /*
    if (clockTick) {
        bufferLeft[0] = 0.4f;
        clockTick = false;
    };
    */
    return isStereo;
}

} // namespace Effect::Delay
