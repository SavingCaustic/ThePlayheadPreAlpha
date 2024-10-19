#pragma once
#include "MidiDriver.h"
#include <iostream>
#include <memory>
#include <string>

// possibly, this is only the midi-in-manager.. dunno.. hotplugging has to be maintained somewhere..

class MidiManager {
  public:
    static constexpr size_t MaxDevices = 5;
    MidiDriver midiDrivers[MaxDevices]; // Fixed array of MIDI drivers

    MidiManager() {
        // Initialize all drivers in a stopped state
        for (auto &driver : midiDrivers) {
            driver.stop(); // Ensure all drivers are in a valid state
        }
    }

    // Lists all available MIDI devices
    std::vector<std::string> getAvailableDevices() {
        RtMidiIn midiIn;
        std::vector<std::string> deviceNames;
        unsigned int nPorts = midiIn.getPortCount();
        for (unsigned int i = 0; i < nPorts; i++) {
            deviceNames.push_back(midiIn.getPortName(i));
        }
        return deviceNames;
    }

    void mountAllDevices() {
        std::vector<std::string> devices;
        devices = getAvailableDevices();
        for (auto &dev : devices) {
            mountDevice(dev);
        }
    }

    // Add a new device and mount it
    bool mountDevice(const std::string &deviceName) {
        for (auto &driver : midiDrivers) {
            // Only try to start an unmounted device
            if (!driver.hasMessages() && driver.start(deviceName)) {
                std::cout << "Mounted device: " << deviceName << std::endl;
                return true;
            }
        }
        std::cerr << "No available slots for new MIDI devices." << std::endl;
        return false;
    }

    // Stop all mounted devices
    void stopAll() {
        for (auto &driver : midiDrivers) {
            driver.stop();
        }
    }

    bool getNextMessage(MidiMessage &newMessage) {
        // maybe optimize this with a flag on incoming midi..
        //  Scan all midi devices for any new messages in their buffers.
        for (auto &driver : midiDrivers) {
            if (driver.isRunning()) {
                if (driver.bufferRead(newMessage)) {
                    return true; // Return if a message is successfully read
                }
            }
        }
        return false; // No messages found
    }
};
