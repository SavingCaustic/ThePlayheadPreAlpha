#include "./PlayerEngine.h"

thread_local AudioHallway audioHallway;

PlayerEngine::PlayerEngine()
    : noiseVolume(0.2f), isWritingMessage(false), hRotator(), objectManager(racks), ccManager(*this) {
    this->rackReceivingMidi = 0; // meh
    this->loadAvg = 0.0f;
    this->hRotator.setTempo(125);
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

void PlayerEngine::bindMessageInQueue(MessageInQueue &hMessageInQueue) {
    messageInQueue = &hMessageInQueue;
}

void PlayerEngine::bindMessageOutQueue(MessageOutQueue &hMessageOutQueue) {
    messageOutQueue = &hMessageOutQueue;
}

void PlayerEngine::bindLoggerQueue(AudioLoggerQueue &hAudioLoggerQueue) {
    audioLoggerQueue = &hAudioLoggerQueue;
}

void PlayerEngine::bindDestructorQueue(Destructor::Queue &hDestructorQueue) {
    // really just a proxy
    objectManager.destructorQueue = &hDestructorQueue;
}

void PlayerEngine::bindProjectSettingsManager(ProjectSettingsManager &psManager) {
    hProjectSettingsManager = &psManager;
}

void PlayerEngine::initAudioHallway() {
    audioHallway.destructorQueueMount(*objectManager.destructorQueue);
    audioHallway.audioQueueMount(*audioLoggerQueue);
    LoggerRec logTemp;
    FORMAT_LOG_MESSAGE(logTemp, LOG_INFO, "If you do good, you see me %s..", "once");
    audioHallway.logMessage(logTemp);
    audioHallwaySetup = true;
}

void PlayerEngine::bindConstructorQueue(Constructor::Queue &hConstructorQueue) {
    // really just a proxy
    objectManager.constructorQueue = &hConstructorQueue;
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
    messageOutQueue->push(message);

    // Release the write lock
    isWritingMessage.store(false, std::memory_order_release);
    return true;
}

float PlayerEngine::getLoadAvg() {
    // this should really be atomic.. and maybe skip altogether..
    return this->loadAvg;
}

void PlayerEngine::renderNextBlock(float *buffer, unsigned long numFrames) {
    // Get the current time in microseconds
    // Calculate time for next frame based on sample rate and numFrames
    if (!audioHallwaySetup) {
        std::cout << "setting up audio hallway." << std::endl;
        initAudioHallway();
    }
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
    // rotate the main wheel so the rest can follow
    bool newEight = this->hRotator.frameTurn();
    if (newEight && this->isPlaying) {
        // we got a new eighth. Increase on all racks(?)
        for (uint8_t i = 0; i < TPH_RACK_COUNT; i++) {
            if (this->racks[i].enabled) {
                // this->racks[i]->hPatternPlayer->incEightCounter();
            }
        }
    }

    if (true) { // this->test) {
        if (hProjectSettingsManager->checkNewSetting()) {
            if (hProjectSettingsManager->checkSingleNewSetting()) {
                // ok fine..
                std::cout << "found new kwy " << hProjectSettingsManager->newSettingKey << " with value of " << hProjectSettingsManager->getNewSetting() << std::endl;
                uint32_t newKey = Utils::Hash::fnv1a(hProjectSettingsManager->newSettingKey);
                std::string *val = hProjectSettingsManager->getNewSetting();
                int intVal;
                switch (newKey) {
                case Utils::Hash::fnv1a_hash("bpm"):
                    intVal = std::stoi(*val);
                    // passing "120" to 120 to the tempo engine - will it work??
                    if (intVal > 40 && intVal < 250) {
                        hRotator.setTempo(intVal, false);
                    }
                    break;
                case Utils::Hash::fnv1a_hash("master_tune"):
                    intVal = std::stoi(*val);
                    if (intVal > 420 && intVal < 460) {
                        AudioMath::setMasterTune(static_cast<float>(intVal));
                    }
                default:
                    std::cout << "unknown project setting " << hProjectSettingsManager->newSettingKey << std::endl;
                }
            } else {
                // so iterate over many new settings.. maybe only if not running - i dunno..
            }
            hProjectSettingsManager->clearCommit();
        }
    }

    if (this->test) {
        return;
    }
    //
    int64_t timeLeftUs = calcTimeLeftUs(nextFrameTime, frameDurationMicroSec);
    if (timeLeftUs > 500) {
        // check if there's any parameter - permanent or not(?) that should be forwarded to a rack module..
        auto optionalMessage = messageInQueue->pop();
        if (optionalMessage) { // Check if a message was retrieved
            newMessage = *optionalMessage;
            std::cout << "New message received," << newMessage.paramName << "value:" << newMessage.paramValue << std::endl;
            // this->sendError(404, "duh message recieved");
            racks[newMessage.rackId].passParamToUnit(
                Rack::stringToUnitType(newMessage.target),
                newMessage.paramName,
                newMessage.paramValue);
            // This echoes back to WS-client, but needs to be elaborated
            // sendMessage(1, "synth", newMessage.paramValue, newMessage.paramName, "yaba daba");
        }
    }

    timeLeftUs = calcTimeLeftUs(nextFrameTime, frameDurationMicroSec);
    if (timeLeftUs > 500) {
        objectManager.process();
    }

    debugCnt++;
    if ((debugCnt & (4096 - 1)) == 0) {
        sendLoadStats(nextFrameTime, frameDurationMicroSec);
    }
}

int64_t PlayerEngine::calcTimeLeftUs(std::chrono::time_point<std::chrono::high_resolution_clock> nextFrameTime, int64_t frameDurationMicroSec) {
    return std::chrono::duration_cast<std::chrono::microseconds>(
               nextFrameTime - std::chrono::high_resolution_clock::now())
        .count();
}

void PlayerEngine::sendLoadStats(std::chrono::time_point<std::chrono::high_resolution_clock> nextFrameTime, int64_t frameDurationMicroSec) {
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

    FORMAT_LOG_MESSAGE(logTemp, LOG_INFO, "Usage percent: %f", this->loadAvg);
    sendAudioLog();

    FORMAT_LOG_MESSAGE(logTemp, LOG_INFO, "Time left (uS): %d", static_cast<int>(timeLeftUs));
    sendAudioLog();
}

void PlayerEngine::sendAudioLog() {
    audioHallway.logMessage(logTemp);
}

std::string PlayerEngine::getSynthParams(int rackId) {
    // this shoud really require rack to be non-playing.. ..or receieved from queue - not directly from endpoint.
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
            for (std::size_t sample = 0; sample < TPH_RACK_RENDER_SIZE; sample++) {
                *(buffer + offset + sample * 2) += racks[i].audioBufferLeft[sample];      // L
                *(buffer + offset + sample * 2 + 1) += racks[i].audioBufferRight[sample]; // R
            }
        }
    }
}

bool PlayerEngine::pollMidiIn() {
    MidiMessage newMessage;
    uint8_t remappedCC = 0;
    // if (!midiManager.)
    this->rackReceivingMidi = 1;
    if (this->rackReceivingMidi >= 0) {
        while (midiManager->getNextMessage(newMessage)) {
            // here, recover note-on with vel 0 to note off. (coule be in midimaager)
            if ((newMessage.cmd & 0xf0) == 0x90 && newMessage.param2 == 0) {
                newMessage.cmd -= 0x10;
            }
            const uint8_t channel = static_cast<uint8_t>(newMessage.cmd) & 0x03; // 0x03 for prototype - 4 racks..
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