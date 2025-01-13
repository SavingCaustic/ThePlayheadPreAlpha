#include "core/utils/WavReader.h"
#include "core/utils/WavWriter.h"
#include <atomic>
#include <iostream>
#include <signal.h>

Utils::WavReader reader;
Utils::WavWriter writer;

int debugWav() {
    reader.open("assets/Synth/Beatnik/lm-2/snare-l.wav");
    writer.open("snare-copy.wav", 48000, 1);

    // Reserve memory for data based on the WAV file's header information
    Utils::WavHeader header;
    if (!reader.getFileInfo(header)) {
        std::cerr << "Failed to read WAV file info" << std::endl;
        return -1;
    }

    // Calculate total samples
    int totalSamples = header.data_length / header.block_align * header.num_channels;

    // Preallocate memory for efficiency
    std::vector<float> data;
    data.reserve(totalSamples);

    // Read data and write to the new file
    if (!reader.returnWavAsFloat(data)) {
        std::cerr << "Failed to load WAV data" << std::endl;
        return -1;
    }
    writer.write(data.data(), data.size());

    reader.close();
    writer.close();
    std::cout << "ok" << std::endl;
    return 0;
}
