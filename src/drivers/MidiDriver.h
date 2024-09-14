#include "../core/PlayerEngine.h"
#include <rtmidi/RtMidi.h>

class MidiDriver {
  public:
    // Constructor accepting a PlayerEngine instance
    MidiDriver(PlayerEngine *engine) : midiIn(nullptr), is_running(false), playerEngine(engine) {}

    ~MidiDriver() {
        stop();
    }

    // Start the MIDI input service
    bool start() {
        try {
            midiIn = new RtMidiIn(RtMidi::Api::LINUX_ALSA);

            // Check available ports
            unsigned int nPorts = midiIn->getPortCount();
            if (nPorts == 0) {
                std::cerr << "No MIDI ports available!" << std::endl;
                delete midiIn;
                midiIn = nullptr;
                return false;
            }

            // Find the port that matches your MIDI device
            std::string deviceName = "Impact LX25+";
            unsigned int selectedPort = -1;
            for (unsigned int i = 0; i < nPorts; i++) {
                std::string portName = midiIn->getPortName(i);
                if (portName.find(deviceName) != std::string::npos) {
                    selectedPort = i;
                    break;
                }
            }

            if (selectedPort == -1) {
                std::cerr << "MIDI device '" << deviceName << "' not found." << std::endl;
                delete midiIn;
                return false;
            }

            // Open the selected port
            midiIn->openPort(selectedPort);

            // Set the callback function
            midiIn->setCallback(&MidiDriver::midiCallback, this);

            is_running = true;
            std::cout << "MIDI service started. Listening for MIDI messages..." << std::endl;
            return true;
        } catch (RtMidiError &error) {
            error.printMessage();
            return false;
        }
    }

    // Stop the MIDI input service
    void stop() {
        if (is_running && midiIn != nullptr) {
            midiIn->cancelCallback();
            delete midiIn;
            midiIn = nullptr;
            is_running = false;
            std::cout << "MIDI service stopped." << std::endl;
        }
    }

    void playerPing() {
        // this is just a temp test since we can't reach playerEngine direcly in main.cpp
        playerEngine->ping();
    }

    void playerSynthSetup() {
        // same here - this should not be here but it's just a mock since the real deal isn't working.
        playerEngine->testRackSetup();
    }

  private:
    static void midiCallback(double deltatime, std::vector<unsigned char> *message, void *userData) {
        MidiDriver *driver = static_cast<MidiDriver *>(userData);
        driver->handleMidiMessage(deltatime, message);
    }

    // Handle the incoming MIDI message
    void handleMidiMessage(double deltatime, std::vector<unsigned char> *message) {
        std::cout << "Received MIDI message: ";
        for (unsigned char byte : *message) {
            std::cout << static_cast<int>(byte) << std::endl;
        }
        std::cout << std::endl;
        // Use the PlayerEngine instance
        if (playerEngine) {
            playerEngine->noteOnDemo();
        }
    }

    RtMidiIn *midiIn;
    bool is_running;
    PlayerEngine *playerEngine; // Instance of PlayerEngine
};
