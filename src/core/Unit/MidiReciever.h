#pragma once
#include <iostream>
#include <cstdint>

class MidiRecieverInterface {
  public:
    virtual void parseMidi(uint8_t cmd, uint8_t param1, uint8_t param2) = 0; // Handle MIDI input
    virtual ~MidiRecieverInterface() = default;
};
