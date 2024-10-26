#pragma once
#include "constants.h"
#include "core/player/PlayerEngine.h"
#include "drivers/AudioDriver.h"
#include <vector>

enum class AudioManagerState { Stopped,
                               AudioOut,
                               AudioInAndOut };

enum CallbackMode { Player,
                    Tuner };

class AudioManager {
  public:
    AudioManager(AudioDriver &driver, PlayerEngine &engine)
        : audioDriverRef(driver), playerEngineRef(engine), state(AudioManagerState::Stopped), callbackMode(Player) {
        // Constructor does not register the callback
        initializeCallback();
    }

    // Method to register the callback function after initialization
    void initializeCallback() {
        audioDriverRef.registerCallback([this](float *in, float *out, unsigned long frameCount) {
            theGreatCallback(in, out, frameCount);
        });
    }

    // Start the audio engine
    bool start() {
        state = AudioManagerState::AudioOut;
        return audioDriverRef.start();
    }

    // Stop the audio engine
    void stop() {
        audioDriverRef.stop();
        state = AudioManagerState::Stopped;
    }

    std::vector<AudioDeviceInfo> getAvailableDevices() {
        return audioDriverRef.getAvailableDevices();
    }

    void mountDevice(int deviceID) {
        if (mountedDeviceID != -1) {
            unmountDevice();
        }
        audioDriverRef.start(deviceID, 0, 2);
        mountedDeviceID = deviceID;
        state = AudioManagerState::AudioOut;
        std::cout << "tring to get this going.." << std::endl;
    }

    void unmountDevice() {
        if (mountedDeviceID != -1) {
            audioDriverRef.stop();
            mountedDeviceID = -1;
            state = AudioManagerState::Stopped;
        }
    }

    void enableAudioInput() {
        if (state == AudioManagerState::AudioOut) {
            audioDriverRef.stop();
            audioDriverRef.start(audioDriverRef.getDeviceID(), 2, 2);
            state = AudioManagerState::AudioInAndOut;
        }
    }

    void disableAudioInput() {
        if (state == AudioManagerState::AudioInAndOut) {
            audioDriverRef.stop();
            audioDriverRef.start(audioDriverRef.getDeviceID(), 0, 2);
            state = AudioManagerState::AudioOut;
        }
    }

    int getMountedDeviceID() {
        return mountedDeviceID;
    }

    void setCallbackMode(CallbackMode mode) {
        callbackMode = mode;
    }

  private:
    void theGreatCallback(float *in, float *out, unsigned long frameCount) {
        if (callbackMode == Player) {
            playerEngineRef.renderNextBlock(out, frameCount);
        } else if (callbackMode == Tuner) {
            // Tuner-specific processing
        }
    }

    CallbackMode callbackMode;
    int mountedDeviceID = -1;
    AudioManagerState state;
    AudioDriver &audioDriverRef;
    PlayerEngine &playerEngineRef;
};
