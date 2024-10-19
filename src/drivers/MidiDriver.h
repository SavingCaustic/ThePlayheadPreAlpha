#pragma once
#include <atomic>
#include <iostream>
#include <rtmidi/RtMidi.h>
#include <vector>

struct MidiMessage {
    uint8_t cmd;
    uint8_t param1;
    uint8_t param2;

    MidiMessage(uint8_t c = 0, uint8_t p1 = 0, uint8_t p2 = 0)
        : cmd(c), param1(p1), param2(p2) {}
};

class MidiDriver {
  public:
    static constexpr size_t BufferSize = 64; // Fixed buffer size

    MidiDriver() : midiIn(nullptr), is_running(false), bufferWriteIndex(0), bufferReadIndex(0),
                   midiInState(0), midiInCmd(0), midiInP1(0), midiInLength(0) {}

    ~MidiDriver() {
        stop();
    }

    bool start(std::string deviceName = "Virtual Keyboard") {
        try {
            midiIn = new RtMidiIn(RtMidi::Api::LINUX_ALSA);

            unsigned int nPorts = midiIn->getPortCount();
            if (nPorts == 0) {
                std::cerr << "No MIDI ports available!" << std::endl;
                delete midiIn;
                midiIn = nullptr;
                return false;
            }

            // std::string deviceName = "Impact LX25+";
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

            midiIn->openPort(selectedPort);
            midiIn->setCallback(&MidiDriver::midiCallback, this);

            is_running = true;
            std::cout << "MIDI service started. Listening for MIDI messages..." << std::endl;
            return true;
        } catch (RtMidiError &error) {
            error.printMessage();
            return false;
        }
    }

    void stop() {
        if (is_running && midiIn != nullptr) {
            midiIn->cancelCallback();
            delete midiIn;
            midiIn = nullptr;
            is_running = false;
            std::cout << "MIDI service stopped." << std::endl;
        }
    }

    bool bufferWrite(const MidiMessage &message) {
        auto current_write = bufferWriteIndex.load(std::memory_order_relaxed);
        auto next_write = (current_write + 1) & (BufferSize - 1);

        if (next_write == bufferReadIndex.load(std::memory_order_acquire)) {
            return false; // Buffer is full
        }

        buffer[current_write] = message;
        bufferWriteIndex.store(next_write, std::memory_order_release);
        return true;
    }

    bool bufferRead(MidiMessage &message) {
        auto current_read = bufferReadIndex.load(std::memory_order_relaxed);
        if (current_read == bufferWriteIndex.load(std::memory_order_acquire)) {
            return false; // Buffer is empty
        }

        message = buffer[current_read];
        bufferReadIndex.store((current_read + 1) & (BufferSize - 1), std::memory_order_release);
        return true;
    }

  private:
    static void midiCallback(double deltatime, std::vector<uint8_t> *message, void *userData) {
        MidiDriver *driver = static_cast<MidiDriver *>(userData);
        driver->handleMidiMessage(message);
    }

    void handleMidiMessage(std::vector<uint8_t> *message) {
        for (uint8_t byte : *message) {
            uint8_t test = 0;
            switch (midiInState) {
            case 0:
                test = byte & 0xf0;
                // Expecting a command byte
                midiInCmd = byte;
                switch (test) {
                case 0x80: // Note Off
                case 0x90: // Note On
                case 0xa0: // Aftertouch
                case 0xb0: // Control Change
                case 0xe0: // Pitch Bend
                    midiInLength = 2;
                    break;
                case 0xc0: // Program Change
                case 0xd0: // Channel Pressure
                    midiInLength = 1;
                    break;
                case 0xf0: // System Exclusive
                    midiInLength = 0;
                    return; // Ignore for now
                default:
                    std::cerr << "Unknown MIDI command" << std::endl;
                    return;
                }
                midiInState = 1; // Expecting first parameter
                break;
                //
            case 1:
                // First parameter
                midiInP1 = byte;
                if (midiInLength == 1) {
                    // Single parameter message
                    if (!bufferWrite(MidiMessage(midiInCmd, midiInP1, 0))) {
                        std::cerr << "Buffer full, unable to write message" << std::endl;
                    }
                    midiInState = 0; // Reset state
                } else {
                    midiInState = 2; // Expecting second parameter
                }
                break;

            case 2:
                // Second parameter
                if (!bufferWrite(MidiMessage(midiInCmd, midiInP1, byte))) {
                    std::cerr << "Buffer full, unable to write message" << std::endl;
                }
                midiInState = 0; // Reset state
                break;

            default:
                std::cerr << "Invalid MIDI state" << std::endl;
                midiInState = 0; // Reset state
                break;
            }
        }
    }

    RtMidiIn *midiIn;
    bool is_running;
    MidiMessage buffer[BufferSize];       // Buffer array holding MidiMessage structs
    std::atomic<size_t> bufferWriteIndex; // Write index (producer side)
    std::atomic<size_t> bufferReadIndex;  // Read index (consumer side)
    uint8_t midiInState;                  // State for parsing MIDI messages
    uint8_t midiInCmd;                    // Last received command byte
    uint8_t midiInP1;                     // Last received parameter 1 byte
    uint8_t midiInLength;                 // Expected length of the message
};
