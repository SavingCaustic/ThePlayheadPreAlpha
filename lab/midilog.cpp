#include <iostream>
#include <vector>
#include <rtmidi/RtMidi.h>

// Callback function to handle incoming MIDI messages
void midiCallback(double deltatime, std::vector<unsigned char> *message, void *userData) {
    std::cout << "Received MIDI message: ";
    for (unsigned char byte : *message) {
        std::cout << static_cast<int>(byte) << " ";
    }
    std::cout << std::endl;
}

int main() {
    try {
        // Create an RtMidiIn object
        RtMidiIn *midiIn = new RtMidiIn(RtMidi::Api::LINUX_ALSA); //new RtMidiIn();
        
        // Check available ports
        unsigned int nPorts = midiIn->getPortCount();
        if (nPorts == 0) {
            std::cerr << "No MIDI ports available!" << std::endl;
            delete midiIn;
            return 1;
        }

        // Open the first available port
        midiIn->openPort(0);

        // Set the callback function
        midiIn->setCallback(&midiCallback);

        // Print information
        std::cout << "Listening for MIDI messages on port 0..." << std::endl;
        
        // Keep the program running to listen for MIDI messages
        std::cout << "Press Enter to exit..." << std::endl;
        std::cin.get();

        // Clean up
        midiIn->cancelCallback();
        delete midiIn;
    } catch (RtMidiError &error) {
        error.printMessage();
        return 1;
    }

    return 0;
}
