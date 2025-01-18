#include "core/utils/WavReader.h"
#include "core/utils/WavWriter.h"
#include <atomic>
#include <iostream>
#include <signal.h>

Utils::WavReader reader;
Utils::WavWriter writer;

int debugWav() {
    reader.open("assets/Synth/Beatnik/samples/lm-2/conga-h.wav");
    writer.open("snare-copy.wav", 48000, 1);

    // Reserve memory for data based on the WAV file's header information
    Utils::WavHeader header;
    if (!reader.getFileInfo(header)) {
        std::cerr << "Failed to read WAV file info" << std::endl;
        return -1;
    }

    // Calculate total samples
    int totalSamples = header.data_length / header.block_align * header.num_channels;

    float *data = new float[totalSamples];

    // Load WAV data directly into the allocated memory
    if (!reader.returnWavAsFloat(data, totalSamples)) {
        std::cerr << "Failed to load WAV data" << std::endl;
        delete[] data; // Free memory on failure
        return -1;
    }

    writer.write(data, totalSamples);

    reader.close();
    writer.close();
    delete data;
    std::cout << "ok" << std::endl;
    return 0;
}
