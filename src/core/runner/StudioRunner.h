#include <algorithm> // for std::find
#include <atomic>
#include <chrono>
#include <drivers/AudioManager.h>
#include <drivers/MidiManager.h>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

class StudioRunner {
  public:
    StudioRunner(MidiManager &midiManagerRef, AudioManager &audioManagerRef)
        : running(false), runnerThread(nullptr), runnerCounter(0),
          midiManager(midiManagerRef), audioManager(audioManagerRef) {
        reset();
    }

    void reset() {
        ServerSettingsManager::jsonRead(settings, "server.json");
    }

    // Starts the thread
    void start() {
        running = true;
        std::cout << "starting studio-runner.." << std::endl;
        runnerThread = std::make_unique<std::thread>(&StudioRunner::run, this);
    }

    // Stops the thread
    void stop() {
        running = false;
        if (runnerThread && runnerThread->joinable()) {
            std::cout << "stopping studio-runner.." << std::endl;
            runnerThread->join(); // Ensure the thread finishes execution
        }
    }

    // Destructor ensures clean-up
    ~StudioRunner() {
        stop();
    }

  private:
    std::atomic<bool> running; // Flag to control the thread's run state
    std::unique_ptr<std::thread> runnerThread;
    int runnerCounter;
    MidiManager &midiManager; // Store MidiManager by reference
    AudioManager &audioManager;
    std::unordered_map<std::string, std::string> settings;

    // Assisting thread function that performs preparations and housekeeping
    void run() {
        while (running) {
            // Perform low-priority operations
            // std::cout << "StudioRunner is running...\n";
            highPrioJobs();
            lowPrioJobs();
            runnerCounter++;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    int lowPrioJobs() {
        int phase = runnerCounter % 8;
        switch (phase) {
        case 0:
            midiHotPlugScan();
            break;
        case 1:
            // hotplugging not supported on running connection so skip it
            // audioHotPlugScan();
            break;
        case 2:
            errOutput();
            break;
        case 3:
            // checkSystem
            break;
        }
        return 0;
    }

    int highPrioJobs() {
        return 0;
    }

    int midiHotPlugScan() {
        // std::cout << "running hotplug scan.. " << std::endl;
        //  1) get list of mounted devices and connected devices.
        std::vector<std::string> availableDevices = midiManager.getAvailableDevices();
        std::vector<std::string> mountedDevices = midiManager.getMountedDevices();
        // 2) iterate over mounted. Any not present in devices => unmount
        for (std::string &device : mountedDevices) {
            if (std::find(availableDevices.begin(), availableDevices.end(), device) == availableDevices.end()) {
                midiManager.unmountDevice(device); // Use midiManager, not sMidiManager
            }
        }
        // 3) iterate over connected. Any not present in mounted => mount
        for (std::string &device : availableDevices) {
            if (std::find(mountedDevices.begin(), mountedDevices.end(), device) == mountedDevices.end()) {
                midiManager.mountDevice(device); // Use midiManager, not sMidiManager
            }
        }
        return 0;
    }

    void discoverDevices() {
        // try hard - getting portaudio to discover newly connected devices..
        Pa_Initialize();
        int numDevices = Pa_GetDeviceCount();
        for (int i = 0; i < numDevices; ++i) {
            const PaDeviceInfo *deviceInfo = Pa_GetDeviceInfo(i);
            std::cout << "Device ID " << i << ": " << deviceInfo->name << std::endl;
        }
        Pa_Terminate();
    }

    int audioHotPlugScan() {
        // disabled - to be removed..
        //  Get list of available device IDs and names
        discoverDevices();
        // hey, i'm the StudioRunner. I need some knowledge about settings. Who feeds me?
        std::string preferredDeviceName = settings["audio_device"];
        std::vector<AudioDeviceInfo> availableDevices = audioManager.getAvailableDevices();
        int mountedDeviceID = audioManager.getMountedDeviceID();
        // debug stuff here..
        /*
        std::cout << "prefered device is " << preferredDeviceName << std::endl;
        std::cout << "mountedDeviceID is " << mountedDeviceID << std::endl;
        std::cout << "list of available devices:" << std::endl;
        // Use a range-based for loop
        for (const AudioDeviceInfo &dr : availableDevices) {
            std::cout << dr.name << std::endl; // Assuming AudioDeviceInfo has a 'first' member
        }
        */
        // TOFIX: maybe something here left to look at..
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
                audioManager.unmountDevice();
                audioManager.mountDevice(-1); // -1 for default device
                return 1;                     // Indicates a switch to default
            }
        } else {
            // If the preferred device is not mounted, check if it has connected
            if (preferredDeviceID != -1) {
                // Preferred device found - switch to it
                std::cout << "Preferred device found, switching to preferred device.\n";
                audioManager.unmountDevice();
                audioManager.mountDevice(preferredDeviceID);
                return 2; // Indicates a switch to preferred device
            }
        }

        // Handle case where the device lacks audio input. I guess this logic should be made my manager, not here..

        return 0; // No change needed
    }

    int errOutput() {
        return 0;
    }

    int checkSystem() {
        return 0;
    }
};
