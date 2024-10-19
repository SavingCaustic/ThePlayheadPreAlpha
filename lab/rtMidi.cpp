#include <iostream>
#include <rtmidi/RtMidi.h>

int main() {
    RtMidiIn *midiIn = new RtMidiIn();

    // You can scan for devices whether the port is open or not
    unsigned int numPorts = midiIn->getPortCount();
    for (unsigned int i = 0; i < numPorts; ++i) {
        std::string portName = midiIn->getPortName(i);
        std::cout << "MIDI input port #" << i << ": " << portName << std::endl;
    }

    // Now you can open a port if needed
    midiIn->openPort(0); // Opens the first available port

    // Later on, you can close the port if no longer needed
    midiIn->closePort();
}
