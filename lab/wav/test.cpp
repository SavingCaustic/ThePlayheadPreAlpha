#include "WavWriter.h"
#include <cmath>
#include <iostream>

int main() {
    // Define parameters for the sine wave
    const double DURATION_SECONDS = 2.0;
    const double FREQUENCY = 440.0; // Frequency of A4 note
    const float VOLUME = 0.5;       // Amplitude of the sine wave

    const int SR = 48000;
    // Calculate the number of samples needed
    int num_chunks = static_cast<int>(SR * DURATION_SECONDS / 64);

    WavWriter wav_writer("sine_wave_2s.wav", SR, 64);
    // Create a WavWriter object and write the waveform data
    if (!wav_writer.isOpen()) {
        std::cerr << "Error: Could not open file for writing.\n";
        return -1;
    }

    // Create a buffer to hold the waveform data
    float waveform[64];
    // Generate the sine wave data
    for (int i = 0; i < num_chunks; ++i) {
        for (int j = 0; j < 64; j++) {
            double t = static_cast<float>(i * 64 + j) / SR; // Time in seconds
            waveform[j] = VOLUME * sin(FREQUENCY * t * 2 * M_PI);
        }
        wav_writer.write(waveform);
    }

    // Close the WAV file
    wav_writer.close();

    std::cout << "Sine wave generated and written to sine_wave_2s.wav\n";
    return 0;
}
