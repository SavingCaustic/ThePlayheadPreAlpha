#pragma once
#include <array>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <string>

class PlayerEngine;

class CCManager {
  public:
    explicit CCManager(PlayerEngine &playerEngine);
    uint8_t scrollerCC;
    uint8_t subScrollerCC = 0;
    uint8_t ccScrollerDials[6];
    uint8_t ccScrollerPosition = 0;    // Current scroller position
    uint8_t ccSubScrollerPosition = 0; // Current scroller position

    void updateMidiSettings(const std::string &strScrollerCC, const std::string &strSubScrollerCC, const std::string &strScrollerDials);

    uint8_t remapCC(uint8_t originalCC, uint8_t param2);

  private:
    PlayerEngine &playerEngine;
};
