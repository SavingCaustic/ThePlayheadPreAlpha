#include "PlayerEngine.h"

PlayerEngine::PlayerEngine()
    : noiseVolume(0.2f), // Other initializations
      isWritingMessage(false),
      hRotator() { // Initialize hRotator
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
    if (audioErrorBuffer->addAudioError(code, message)) {
        // std::cout << "wrote error to audioErrorLog" << std::endl;
    } else {
        std::cout << "error log was full" << std::endl;
    }
}

void PlayerEngine::testRackSetup() {
    this->setupRackWithSynth(0, "DummySin");
}

bool PlayerEngine::setupRackWithSynth(int rackId, const std::string &synthName) {
    // Check if the rack already exists
    racks[rackId].setSynth(synthName);
    //   Now, setup the synth for the rack
    racks[rackId].setEffect("Chorus");
    // racks[rackId].setEffect("Delay", 2);
    //  to be improved..
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
    double frameDurationMicroSec = (numFrames * 1'000'000.0) * (1 / TPH_DSP_SR);
    auto nextFrameTime = std::chrono::high_resolution_clock::now() + std::chrono::microseconds(static_cast<long long>(frameDurationMicroSec));
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
    //
    auto endTime = std::chrono::high_resolution_clock::now();
    auto timeLeftUs = std::chrono::duration_cast<std::chrono::microseconds>(nextFrameTime - endTime).count();
    // Time used for rendering in microseconds
    auto timeUsedUs = frameDurationMicroSec - static_cast<double>(timeLeftUs);
    // Clamp the timeUsedUs to avoid negative values (if timeLeftUs is greater than frameDurationMicroSec)
    timeUsedUs = std::max(0.0, timeUsedUs);
    // Calculate raw load average as the ratio of time used to frame duration
    double rawLoad = timeUsedUs / frameDurationMicroSec;
    // Smoothing with a factor (alpha), for example, alpha = 0.1 for smooth update
    constexpr double alpha = 0.1;
    this->loadAvg = (alpha * rawLoad) + ((1 - alpha) * this->loadAvg);
    // this code not working so fake it..
    if (true | timeLeftUs > 1500) {
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

std::string PlayerEngine::getSynthParams(int rackId) {
    // this shoud really require rack to be non-playing..
    return this->racks[rackId].synth->getParamDefsAsJson();
}

void PlayerEngine::sumToMaster(float *buffer, unsigned long numFrames, int outer) {
    int offset = outer * 2 * TPH_RACK_RENDER_SIZE; // Offset in the master buffer

    for (std::size_t i = 0; i < TPH_RACK_COUNT; ++i) {
        if (racks[i].enabled) {
            for (std::size_t sample = 0; sample < TPH_RACK_RENDER_SIZE * 2; sample += 2) {
                *(buffer + offset + sample) = racks[i].audioBuffer[sample];
                *(buffer + offset + sample + 1) = racks[i].audioBuffer[sample + 1];
            }
        }
    }
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

bool PlayerEngine::pollMidiIn() {
    MidiMessage newMessage;
    if (this->rackReceivingMidi >= 0) {
        while (midiManager->getNextMessage(newMessage)) {
            racks[this->rackReceivingMidi].parseMidi(newMessage.cmd, newMessage.param1, newMessage.param2);
            this->sendError(200, "midi recieved");
            if (newMessage.cmd == 0x90)
                sendMessage(1, "synth", newMessage.param1, "note on", "see this? :)");
        }
    }
    return true; // means nothing.. Return the result of getMessage
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