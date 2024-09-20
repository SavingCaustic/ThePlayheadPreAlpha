#pragma once
#include <string>
#include <vector>

class SynthInterface {
  public:
    virtual ~SynthInterface() = default;

    // Pure virtual methods that need to be implemented by derived classes
    virtual void reset() = 0;                                       // Reset the synth to its default state
    virtual void parseMidi(char cmd, char param1, char param2) = 0; // Handle MIDI input
    virtual bool renderNextBlock() = 0;                             // Process the next audio block
    // experimenting with moving push to synth..
    virtual bool pushMyParam(const std::string &name, float val) = 0;
    virtual void pushStrParam(const std::string &name, int val) = 0;

    // Optionally, you can add methods to interact with parameters if needed
};
