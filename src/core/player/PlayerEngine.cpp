#include "./PlayerEngine.h"
#include "ErrorWriter.h"
#include "chrono"

PlayerEngine::PlayerEngine()
    : noiseVolume(0.2f), isWritingMessage(false), hRotator(), errorWriter_(*this) {
    this->rackReceivingMidi = 0; // meh
    this->loadAvg = 0.0f;
}

void PlayerEngine::reset() {
    noiseVolume = 0.2f;
}

void PlayerEngine::ping() {
    std::cout << "ping from player engine" << std::endl;
}

void PlayerEngine::doReset() {
    std::cout << "Resetting player engine" << std::endl;
    reset(); // Calls the instance method
}

void PlayerEngine::initializeRacks() {
    for (int i = 0; i < TPH_RACK_COUNT; ++i) {
        racks[i].setPlayerEngine(*this);
        racks[i].setErrorWriter(errorWriter_); // Pass the ErrorWriter reference
    }
}

void PlayerEngine::bindMessageInBuffer(MessageInBuffer &hMessageInBuffer) {
    messageInBuffer = &hMessageInBuffer;
}

void PlayerEngine::bindMessageOutBuffer(MessageOutBuffer &hMessageOutBuffer) {
    messageOutBuffer = &hMessageOutBuffer;
}

void PlayerEngine::bindErrorBuffer(AudioErrorBuffer &hAudioErrorBuffer) {
    audioErrorBuffer = &hAudioErrorBuffer;
}

void PlayerEngine::bindDestructorBuffer(Destructor::Queue &hDestructorBuffer) {
    destructorBuffer = &hDestructorBuffer;
}

void PlayerEngine::bindMidiManager(MidiManager &hMidiManager) {
    midiManager = &hMidiManager;
}

bool PlayerEngine::sendMessage(int rackId, const char *target, float paramValue, const char *paramName, const char *paramLabel) {
    if (isWritingMessage.exchange(true, std::memory_order_acquire)) {
        // Return false to indicate the message couldn't be sent
        return false;
    }

    MessageOut message;

    message.rackId = rackId;
    message.paramValue = paramValue;

    // Safely copy the strings
    strncpy(message.target, target, msgOutTargetSize - 1);
    message.target[msgOutTargetSize - 1] = '\0';

    strncpy(message.paramName, paramName, msgOutParamNameSize - 1);
    message.paramName[msgOutParamNameSize - 1] = '\0';

    strncpy(message.paramLabel, paramLabel, msgOutParamLabelSize - 1);
    message.paramLabel[msgOutParamLabelSize - 1] = '\0';

    // Push the message to the queue
    messageOutBuffer->push(message);

    // Release the write lock
    isWritingMessage.store(false, std::memory_order_release);
    return true;
}

void PlayerEngine::sendError(int code, const std::string &message) {
    // dunno if this should be kept. But still, units have to be context-aware..
    audioErrorBuffer->addAudioError(code, message);
}

bool PlayerEngine::destroySynth(int rackID) {
    Destructor::Record record;
    record.ptr = racks[rackID].synth;
    record.deleter = [](void *ptr) { delete static_cast<SynthBase *>(ptr); }; // Create deleter for SynthBase
    // Push the record to the destructor queue
    if (!destructorBuffer->push(record)) {
        std::cout << "Destructor queue is full, could not enqueue the synth to be deleted." << std::endl;
    }
    // std::cout << "destroying synth (inside audio-thread)" << std::endl;
    // delete racks[rackID].synth; // Clean up the old synth
    racks[rackID].synth = nullptr;
    racks[rackID].enabled = false; // Disable the rack if no synth
    return true;
}

bool PlayerEngine::loadSynth(SynthBase *&newSynth, int rackID) {
    // bool result = racks[rackID].setSynth(synth);
    if (racks[rackID].synth) {
        // Create a Record containing the pointer and deleter
        destroySynth(rackID);
    }
    // now setup
    racks[rackID].synth = newSynth;
    std::cout << "Binding buffers for synth in rack" << std::endl;
    racks[rackID].synth->bindBuffers(racks[rackID].audioBuffer.data(), racks[rackID].audioBuffer.size()); // Bind the buffer here
    racks[rackID].enabled = true;                                                                         // Mark the rack as enabled
    // Reset the caller's pointer to avoid accidental reuse
    newSynth = nullptr;
    return true;
}

bool PlayerEngine::loadEffect(EffectBase *&newEffect, int rackID, int effectSlot) {
    std::cout << "at loadEffects" << std::endl;
    EffectInterface **effectTarget = nullptr;
    if (effectSlot == 1) {
        effectTarget = &racks[rackID].effect1;
    } else {
        effectTarget = &racks[rackID].effect2;
    }

    if (*effectTarget) {
        delete *effectTarget;
        *effectTarget = nullptr; // Avoid dangling pointer
    }

    if (newEffect) {
        std::cout << "yes new effect" << std::endl;
        *effectTarget = newEffect;
        (*effectTarget)->bindBuffers(racks[rackID].audioBuffer.data(), racks[rackID].audioBuffer.size());
        // racks[rackID].enabled = true; // Mark the rack as enabled
    } else {
        // racks[rackID].enabled = false; // Disable the rack if no synth
    }

    // Reset the caller's pointer to avoid accidental reuse
    newEffect = nullptr;
    return true;
}

bool PlayerEngine::setupRackWithSynth(int rackId, const std::string &synthName) {
    // Check if the rack already exists
    racks[rackId].setSynthFromStr(synthName);
    sendError(200, "real audio error hello");
    //   Now, setup the synth for the rack
    // racks[rackId].setEffect("Delay"); // Chorus
    // racks[rackId].setEffect("Delay", 2);
    //   to be improved..
    rackReceivingMidi = 0;
    return false;
}

float PlayerEngine::getLoadAvg() {
    // this should really be atomic..
    return this->loadAvg;
}

void PlayerEngine::renderNextBlock(float *buffer, unsigned long numFrames) {
    // Get the current time in microseconds
    // Calculate time for next frame based on sample rate and numFrames
    int64_t frameDurationMicroSec = static_cast<long>(numFrames * (1'000'000.0 / TPH_DSP_SR));
    std::chrono::time_point nextFrameTime = std::chrono::high_resolution_clock::now() + std::chrono::microseconds(frameDurationMicroSec);

    //
    if (clockReset) {
        clockResetMethod(); // Reset the clock (requested by play- or stop-command)
    }
    int outerCnt = TPH_AUDIO_BUFFER_SIZE / TPH_RACK_RENDER_SIZE;
    for (int outer = 0; outer < outerCnt; ++outer) {
        pollMidiIn(); // large audio buffers is bad for midi-in accuracy. Fly low.
        turnRackAndRender();
        sumToMaster(buffer, numFrames, outer);
    }
    int64_t timeLeftUs = sendLoadStats(nextFrameTime, frameDurationMicroSec);

    // this code not working so fake it..
    if (timeLeftUs > 500) {
        // check if there's any parameter - permanent or not(?) that should be forwarded to a rack module..
        auto optionalMessage = messageInBuffer->pop();
        if (optionalMessage) { // Check if a message was retrieved
            newMessage = *optionalMessage;
            std::cout << "New message received," << newMessage.paramName << "value:" << newMessage.paramValue << std::endl;
            // this->sendError(404, "duh message recieved");
            racks[newMessage.rackId].passParamToUnit(
                Rack::stringToUnitType(newMessage.target),
                newMessage.paramName,
                newMessage.paramValue);
            sendMessage(1, "synth", newMessage.paramValue, newMessage.paramName, "yaba daba");
        }
    }
}

int64_t PlayerEngine::sendLoadStats(std::chrono::time_point<std::chrono::high_resolution_clock> nextFrameTime, int64_t frameDurationMicroSec) {
    //
    std::chrono::time_point endTime = std::chrono::high_resolution_clock::now();
    int64_t timeLeftUs = std::chrono::duration_cast<std::chrono::microseconds>(nextFrameTime - endTime).count();

    // Time used for rendering in microseconds
    int64_t timeUsedUs = frameDurationMicroSec - timeLeftUs;

    // Clamp the timeUsedUs to avoid negative values (if timeLeftUs is greater than frameDurationMicroSec)
    timeUsedUs = std::max<int64_t>(0, timeUsedUs); // Ensure no negative values

    // Calculate raw load average as the ratio of time used to frame duration (as a float for precision)
    double rawLoad = (100.0 * static_cast<double>(timeUsedUs)) / static_cast<double>(frameDurationMicroSec);

    // Smoothing with a factor (alpha), for example, alpha = 0.1 for smooth update
    constexpr double alpha = 0.1;

    // Update loadAvg with smoothing
    this->loadAvg = (alpha * rawLoad) + ((1 - alpha) * this->loadAvg);

    debugCnt++;
    if ((debugCnt & (1024 - 1)) == 0) {
        sendError(100, "Stat: " + std::to_string(this->loadAvg));
        sendError(105, "TimeLeftUs: " + std::to_string(timeLeftUs));
    }
    return timeLeftUs;
}

std::string PlayerEngine::getSynthParams(int rackId) {
    // this shoud really require rack to be non-playing..
    return this->racks[rackId].synth->getParamDefsAsJSON();
}

void PlayerEngine::sumToMaster(float *buffer, unsigned long numFrames, int outer) {
    int offset = outer * 2 * TPH_RACK_RENDER_SIZE; // Offset in the master buffer
    for (std::size_t sample = 0; sample < TPH_RACK_RENDER_SIZE * 2; sample += 2) {
        *(buffer + sample) = 0;     // L
        *(buffer + sample + 1) = 0; // R
    }

    for (std::size_t i = 0; i < TPH_RACK_COUNT; ++i) {
        if (racks[i].enabled) {
            for (std::size_t sample = 0; sample < TPH_RACK_RENDER_SIZE * 2; sample += 2) {
                *(buffer + offset + sample) += racks[i].audioBuffer[sample];         // L
                *(buffer + offset + sample + 1) += racks[i].audioBuffer[sample + 1]; // R
            }
        }
    }
}

bool PlayerEngine::pollMidiIn() {
    MidiMessage newMessage;
    u_int8_t remappedCC = 0;
    this->rackReceivingMidi = 1;
    if (this->rackReceivingMidi >= 0) {
        while (midiManager->getNextMessage(newMessage)) {
            // here, recover note-on with vel 0 to note off. (coule be in midimaager)
            if ((newMessage.cmd & 0xf0) == 0x90 && newMessage.param2 == 0) {
                newMessage.cmd -= 0x10;
            }
            const u_int8_t channel = static_cast<u_int8_t>(newMessage.cmd) & 0x03; // 0x03 for prototype - 4 racks..
            if (racks[channel].enabled) {
                // if cc-cmd, see if it should be re-routed.
                if ((newMessage.cmd & 0xf0) == 0xb0) { // note parenthesis!!
                    remappedCC = ccManager.remapCC(newMessage.param1, newMessage.param2);
                    newMessage.param1 = remappedCC;
                }
                if (newMessage.param1 != 255) { // scroller dialed, so surpress!
                    racks[channel].parseMidi(newMessage.cmd, newMessage.param1, newMessage.param2);
                }
            }
            if (newMessage.cmd == 0x90)
                sendMessage(1, "synth", newMessage.param1, "note on", "see this? :)");
        }
    }
    return true; // means nothing.. Return the result of getMessage
}

void PlayerEngine::clockResetMethod() {
    hRotator.pulse = 0; // Reset the rotator's pulse

    for (std::size_t i = 0; i < TPH_RACK_COUNT; ++i) {
        if (racks[i].enabled) {
            racks[i].clockReset(); // Reset each rack's clock
        }
    }

    clockReset = false; // Reset the clockReset flag
}

void PlayerEngine::turnRackAndRender() {
    for (std::size_t i = 0; i < TPH_RACK_COUNT; ++i) {
        if (racks[i].enabled) {                     // Check if the rack is initialized (i.e., not null)
            racks[i].probeNewClock(hRotator.pulse); // Call probeNewClock

            if (isPlaying) {
                racks[i].probeNewTick(hRotator.pulse); // Call probeNewTick if playing
            }

            racks[i].render(1); // Call render
        }
    }
}