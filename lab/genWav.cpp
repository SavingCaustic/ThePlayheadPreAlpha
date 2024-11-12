#include "../src/core/ext/dr_wav.h" // Include dr_wav header
#include <iostream>
#include <math.h>
#include <vector>

class WavWriter {
  private:
    drwav wav; // drwav object for handling WAV files
    std::string filename;
    uint32_t sampleRate;
    uint16_t numChannels;
    uint16_t bitsPerSample;
    std::vector<int16_t> buffer; // Buffer to hold audio samples

  public:
    WavWriter(const std::string &file, uint32_t sampleRate = 48000, uint16_t numChannels = 2, uint16_t bitsPerSample = 16)
        : filename(file), sampleRate(sampleRate), numChannels(numChannels), bitsPerSample(bitsPerSample) {

        // Open WAV file for writing (overwrite mode)
        drwav_data_format format;
        format.container = drwav_container_riff; // Use the standard RIFF container
        format.format = DR_WAVE_FORMAT_PCM;      // PCM format
        format.channels = numChannels;
        format.sampleRate = sampleRate;
        format.bitsPerSample = bitsPerSample;

        if (!drwav_init_file_write(&wav, filename.c_str(), &format, NULL)) {
            std::cerr << "Failed to initialize WAV file for writing." << std::endl;
            exit(1);
        }

        std::cout << "WAV file opened for writing: " << filename << std::endl;
    }

    ~WavWriter() {
        // Close the file when done
        // if (wav.file != nullptr) {
        drwav_uninit(&wav);
        //}
    }

    void appendSamples(const std::vector<int16_t> &samples) {
        // Append the samples to the buffer and write in chunks
        buffer.insert(buffer.end(), samples.begin(), samples.end());

        // Write samples in chunks to avoid large memory usage
        const size_t chunkSize = 1024; // Arbitrary chunk size
        while (buffer.size() >= chunkSize) {
            drwav_write_pcm_frames(&wav, chunkSize, buffer.data());
            buffer.erase(buffer.begin(), buffer.begin() + chunkSize); // Remove written samples
        }
    }

    void flush() {
        // Write any remaining samples in the buffer
        if (!buffer.empty()) {
            drwav_write_pcm_frames(&wav, buffer.size(), buffer.data());
            buffer.clear();
        }
    }
};

int main() {
    WavWriter writer("output.wav");

    // Generate a simple sine wave signal and add samples in chunks
    const uint32_t sampleRate = 48000;
    const uint32_t numChannels = 2;
    const float frequency = 440.0f;          // A4 note (440Hz)
    const uint32_t numSamplesPerChunk = 480; // Number of samples to write per chunk
    const size_t numChunks = 100;            // Number of chunks to write

    const float amplitude = 30000.0f; // Max value for 16-bit PCM

    for (size_t chunkIndex = 0; chunkIndex < numChunks; ++chunkIndex) {
        std::vector<int16_t> chunkSamples;

        for (size_t i = 0; i < numSamplesPerChunk; ++i) {
            // Generate a sine wave sample
            float sample = amplitude * std::sin(2 * 3.14159f * frequency * (chunkIndex * numSamplesPerChunk + i) / sampleRate);
            int16_t intSample = static_cast<int16_t>(sample);

            // Add samples for both channels (stereo)
            chunkSamples.push_back(intSample); // Left channel
            chunkSamples.push_back(intSample); // Right channel
        }

        writer.appendSamples(chunkSamples); // Append generated chunk to the WAV file
    }

    writer.flush(); // Make sure all data is written

    std::cout << "WAV file writing complete." << std::endl;
    return 0;
}
