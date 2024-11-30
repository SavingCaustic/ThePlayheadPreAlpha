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
    MidiDriver(const MidiDriver &) = delete;
    MidiDriver &operator=(const MidiDriver &) = delete;

    static constexpr size_t BufferSize = 64; // Fixed buffer size

    MidiDriver() : midiIn(nullptr), is_running(false), bufferWriteIndex(0), bufferReadIndex(0), midiInState(0), midiInCmd(0), midiInP1(0), midiInLength(0) {}

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

            this->deviceName = deviceName;
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
            deviceName.clear();
            std::cout << "MIDI service stopped." << std::endl;
        }
    }

    bool isRunning() {
        return is_running;
    }

    std::string getDeviceName() {
        return this->deviceName;
    }

    bool bufferWrite(const MidiMessage &message) {
        auto current_write = bufferWriteIndex.load(std::memory_order_relaxed);
        auto next_write = (current_write + 1) % BufferSize;

        if (next_write == bufferReadIndex.load(std::memory_order_acquire)) {
            return false; // Buffer is full
        }
        // std::cout << "incoming.." << std::endl;
        buffer[current_write] = message;
        bufferWriteIndex.store(next_write, std::memory_order_release);
        return true;
    }

    bool bufferRead(MidiMessage &message) {
        auto current_read = bufferReadIndex.load(std::memory_order_relaxed);
        if (current_read == bufferWriteIndex.load(std::memory_order_acquire)) {
            return false; // Buffer is empty
        }
        // std::cout << "reading midi.." << std::endl;

        message = buffer[current_read];
        bufferReadIndex.store((current_read + 1) % BufferSize, std::memory_order_release);
        return true;
    }

    bool hasMessages() const {
        return bufferReadIndex.load(std::memory_order_acquire) != bufferWriteIndex.load(std::memory_order_acquire);
    }

  private:
    static void midiCallback(double deltatime, std::vector<uint8_t> *message, void *userData) {
        MidiDriver *driver = static_cast<MidiDriver *>(userData);
        driver->handleMidiMessage(message);
    }

    void handleMidiMessage(std::vector<uint8_t> *message) {
        for (uint8_t byte : *message) {
            switch (midiInState) {
            case 0:
                midiInCmd = byte;
                midiInLength = (byte & 0xf0) == 0xC0 || (byte & 0xf0) == 0xD0 ? 1 : ((byte & 0xf0) == 0xF0 ? 0 : 2);
                midiInState = midiInLength > 0 ? 1 : 0; // Move to next state
                break;
            case 1:
                midiInP1 = byte;
                if (midiInLength == 1) {
                    bufferWrite(MidiMessage(midiInCmd, midiInP1, 0));
                }
                midiInState = 2; // Expecting second parameter
                break;
            case 2:
                bufferWrite(MidiMessage(midiInCmd, midiInP1, byte));
                midiInState = 0; // Reset state
                break;
            default:
                midiInState = 0; // Reset state on error
                break;
            }
        }
    }

    RtMidiIn *midiIn;
    bool is_running;
    std::string deviceName;
    MidiMessage buffer[BufferSize];       // Buffer array holding MidiMessage structs
    std::atomic<size_t> bufferWriteIndex; // Write index (producer side)
    std::atomic<size_t> bufferReadIndex;  // Read index (consumer side)
    uint8_t midiInState;                  // State for parsing MIDI messages
    uint8_t midiInCmd;                    // Last received command byte
    uint8_t midiInP1;                     // Last received parameter 1 byte
    uint8_t midiInLength;                 // Expected length of the message
};
