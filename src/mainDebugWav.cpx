#include "core/utils/WavReader.h"
#include "core/utils/WavWriter.h"
#include <atomic>
#include <iostream>
#include <signal.h>

std::atomic<bool> shutdown_flag(false);

// Custom signal handler. Working well.
void signal_handler(int signal) {
    if (signal == SIGINT) {
        std::cout << "Caught SIGINT (Ctrl+C), setting shutdown flag..." << std::endl;
        shutdown_flag = true;
    }
}

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