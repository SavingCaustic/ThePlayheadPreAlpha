#include "core/utils/WavReader.h"
#include "core/utils/WavWriter.h"
#include <atomic>
#include <iostream>
#include <signal.h>

Utils::WavReader reader;
Utils::WavWriter writer;

int debugWav() {
    // snare-m is MONO
    reader.open("assets/Synth/Beatnik/samples/lm-2/snare-m.wav");
    // reader.open("assets/Synth/Beatnik/samples/HR16A/Alesis_HR16A_43.wav");
    writer.open("snare-m-copy.wav", 48000, 1);

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
    // ah this no work any more since data is interleaved..
    writer.writeMono(data, totalSamples);

    reader.close();
    writer.close();
    delete data;
    std::cout << "ok" << std::endl;
    return 0;
}
