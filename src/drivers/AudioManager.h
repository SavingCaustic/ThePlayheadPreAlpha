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

    int mountPreferedOrDefault(const std::string preferredDeviceName) {
        std::vector<AudioDeviceInfo> availableDevices = getAvailableDevices();
        int mountedDeviceID = getMountedDeviceID();
        //  Find the ID of the preferred device by name
        int preferredDeviceID = -1;
        for (const auto &device : availableDevices) {
            if (device.name == preferredDeviceName) {
                preferredDeviceID = device.id;
                break;
            }
        }
        // ok, now if prefered device was found, preferredDeviceID is set.
        // as backup, use 'default' as preferredDeviceID
        if (preferredDeviceID == -1) {
            // go find default..
            for (const auto &device : availableDevices) {
                if (device.name == "default") {
                    preferredDeviceID = device.id;
                    break;
                }
            }
        }

        // Check if the preferred device is already mounted
        if (mountedDeviceID == preferredDeviceID) {
            // If the preferred device is mounted, check if it's still connected
            bool preferredDeviceConnected = std::any_of(
                availableDevices.begin(), availableDevices.end(),
                [preferredDeviceID](const AudioDeviceInfo &dev) { return dev.id == preferredDeviceID; });
            // if all good, early exit (above), if not, re-route..

            if (!preferredDeviceConnected) {
                // Preferred device lost - revert to default
                std::cout << "Preferred device lost, reverting to default.\n";
                unmountDevice();
                mountDevice(-1); // -1 for default device
                return 1;        // Indicates a switch to default
            }
        } else {
            // If the preferred device is not mounted, check if it has connected
            if (preferredDeviceID != -1) {
                // Preferred device found - switch to it
                std::cout << "Preferred device found, switching to preferred device.\n";
                unmountDevice();
                mountDevice(preferredDeviceID);
                return 2; // Indicates a switch to preferred device
            }
        }
        return 0;
    }

    void setCallbackMode(CallbackMode mode) {
        callbackMode = mode;
    }

  private:
    void theGreatCallback(float *in, float *out, unsigned long frameCount) {
        if (callbackMode == Player) {
            // yeah. we're taking over resposibility of frameCount.
            playerEngineRef.renderNextBlock(out, frameCount); // TPH_AUDIO_BUFFER_SIZE
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
