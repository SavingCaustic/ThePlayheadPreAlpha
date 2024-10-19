#pragma once
#include "MidiDriver.h"
#include <iostream>
#include <memory>
#include <rtmidi/RtMidi.h>
#include <string>
#include <vector>

class MidiManager {
  public:
    std::vector<MidiDriver> midiDrivers; // Hold multiple MIDI drivers
    std::unique_ptr<RtMidiIn> midiIn;    // Pointer to RtMidiIn for device management

    MidiManager() : midiIn(std::make_unique<RtMidiIn>()) {}

    // Lists all available MIDI devices
    void listDevices() {
        unsigned int nPorts = midiIn->getPortCount();
        if (nPorts == 0) {
            std::cout << "No MIDI ports available!" << std::endl;
            return;
        }

        std::cout << "Available MIDI ports:" << std::endl;
        for (unsigned int i = 0; i < nPorts; i++) {
            std::string portName = midiIn->getPortName(i);
            std::cout << "  Port " << i << ": " << portName << std::endl;
        }
    }

    // Add a new device and mount it
    bool mountDevice(const std::string &deviceName) {
        MidiDriver driver;
        if (driver.start(deviceName)) {
            midiDrivers.push_back(std::move(driver)); // Move the driver into the vector
            return true;
        }
        return false;
    }

    // Stop all mounted devices
    void stopAll() {
        for (auto &driver : midiDrivers) {
            driver.stop();
        }
        midiDrivers.clear(); // Clear the vector after stopping all drivers
    }

    MidiMessage getNextMessage() {
        // scan all midi devices for any new messages in their buffers.
        // this function is called by the audio thread
    }
};
