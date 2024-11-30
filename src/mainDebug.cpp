#include <Synth/Monolith/MonolithModel.h>
#include <Synth/SynthFactory.h>
#include <atomic>
#include <core/player/Rack.h>
#include <iostream>
#include <signal.h>

#define DEBUG_MODE 1

std::atomic<bool> shutdown_flag(false);

// Custom signal handler. Working well.
void signal_handler(int signal) {
    if (signal == SIGINT) {
        std::cout << "Caught SIGINT (Ctrl+C), setting shutdown flag..." << std::endl;
        shutdown_flag = true;
    }
}

int main() {

    float audioBuffer[64];
    const std::size_t bufferSize = 64;

    SynthInstance *synth = nullptr;

    // 1 this code works - but has to go..
    synth = new Synth::Monolith::Model();
    synth->bindBuffers(audioBuffer, bufferSize);
    delete synth;
    synth = nullptr;

    // 2 using factory doesn't work - why?
    SynthFactory::setupSynth(synth, "Monolith");
    SynthFactory::patchLoad(synth, "Portabello");
    synth->bindBuffers(audioBuffer, bufferSize);
    delete synth;

    // this method probably called from playerEngine later..

    // setting up lut's and all..
    // Admin::Synth::loadPatch(synth, "Monolith", "Perfect sine");
    //? ? Admin::Rack::mountSynth(synth, 5);
    // queueRackMount(5, "Synth", synth);
    return 0;
}
