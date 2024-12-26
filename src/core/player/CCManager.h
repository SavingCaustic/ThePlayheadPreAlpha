#pragma once
#include <array>
#include <cmath>
#include <iostream>
#include <string>

class CCManager {
  public:
    uint8_t scrollerCC;
    uint8_t subScrollerCC = 0;
    uint8_t ccScrollerDials[6];
    uint8_t ccScrollerPosition = 0;    // Current scroller position
    uint8_t ccSubScrollerPosition = 0; // Current scroller position

    // uint8_t process(uint8_t param1, uint8_t param2) {
    // }

    void updateMidiSettings(const std::string &strScrollerCC, const std::string &strSubScrollerCC, const std::string &strScrollerDials) {
        this->scrollerCC = std::stoi(strScrollerCC);
        this->subScrollerCC = std::stoi(strSubScrollerCC);
        std::cout << "setting scrolerCC to " << this->scrollerCC << std::endl;
        // Initialize the scroller dials to zero
        std::fill(std::begin(ccScrollerDials), std::end(ccScrollerDials), 0);

        // Parse the comma-separated scroller dial values
        size_t start = 0, end = 0;
        int index = 0;

        while ((end = strScrollerDials.find(',', start)) != std::string::npos && index < 7) {
            ccScrollerDials[index++] = std::stoi(strScrollerDials.substr(start, end - start));
            start = end + 1;
        }

        // Add the last value (or only value if no commas were found)
        if (index < 7 && start < strScrollerDials.size()) {
            ccScrollerDials[index++] = std::stoi(strScrollerDials.substr(start));
        }
    }

    uint8_t remapCC(uint8_t originalCC, uint8_t param2) {
        // Check if the CC corresponds to a pot
        if (originalCC == scrollerCC) {
            uint8_t testScroller = round(param2 * (6.0f / 127.0f));
            if ((testScroller & 0x01) == 0x00) {
                // at value (not threshold), now shift.
                testScroller = testScroller >> 1;
                if (ccScrollerPosition != testScroller) {
                    std::cout << "setting pager to " << static_cast<int>(testScroller) << std::endl;
                    ccScrollerPosition = testScroller;
                }
            }
            return 255; // surpress later processing
        }
        // do the same for subscroller
        if (originalCC == subScrollerCC) {
            uint8_t testScroller = round(param2 * (6.0f / 127.0f));
            if ((testScroller & 0x01) == 0x00) {
                // at value (not threshold), now shift.
                testScroller = testScroller >> 1;
                if (ccSubScrollerPosition != testScroller) {
                    std::cout << "setting sub-pager to " << static_cast<int>(testScroller) << std::endl;
                    ccSubScrollerPosition = testScroller;
                }
            }
            return 255; // surpress later processing
        }

        for (int i = 0; i < 6; i++) {
            if (originalCC == ccScrollerDials[i]) {
                // ok, a bit more complicated.. if scroller pos 0 or 1 (synth):
                uint8_t newCC;
                switch (ccScrollerPosition) {
                case 0:
                case 1:
                    // synth stuff.. CC16 - 16+8*6=64 (63)
                    newCC = 16 + (ccScrollerPosition * 4 + ccSubScrollerPosition) * 6 + i;
                    std::cout << "routed CC:" << static_cast<int>(newCC) << std::endl;
                    break;
                case 2:
                    // eventors & effects ev1:72-77, ev2:78-83, ef1: 84-89, ef2:90-95
                    newCC = 24 + (2 * 4 + ccSubScrollerPosition) * 6 + i;
                    break;
                case 3:
                    // emittor and ? patch-assigned?
                    newCC = 24 + (3 * 4 + ccSubScrollerPosition) * 6 + i;
                    break;
                }
                return newCC;
            }
        }
        return originalCC; // No remapping needed
    }
};