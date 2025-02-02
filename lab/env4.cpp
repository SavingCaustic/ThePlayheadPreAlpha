#include <chrono>
#include <cmath>
#include <iostream>

// Define constants
constexpr int SR = 48000;
constexpr int BLOCK_SIZE = 64;

int main() {
    float oldAmp = 0.2f;
    float newAmp = 0.8f;
    float myAmp[BLOCK_SIZE];
    float step = (newAmp - oldAmp) / BLOCK_SIZE;
    float acc = oldAmp;
    float factor = 0;
    for (int i = 0; i < BLOCK_SIZE; i++) {
        factor = 2.0f - abs(32 - i) / 16.0f;
        acc += step * factor;
        myAmp[i] = acc;
    }

    return 0;
}