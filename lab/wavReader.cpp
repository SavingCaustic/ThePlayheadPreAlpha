#include "../src/core/utils/WavReader.h"
#include "../src/core/utils/WavWriter.h"
#include <iostream>

Utils::WavReader reader;
Utils::WavWriter writer;

int main() {
    reader.open("snare-l.wav");
    writer.open("snare-copy.wav", 48000, 1);
    std::vector<float> data;
    reader.returnWavAsFloat(data);
    writer.write(data.data(), data.size());
    reader.close();
    writer.close();
    std::cout << "ok" << std::endl;
    return 0;
}