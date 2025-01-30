#include "core/hallways/FactoryHallway.h"
#include "mainDebugBeatnik.cpp"
#include "mainDebugPatchLoad.cpp"
#include "mainDebugSubreal.cpp"
#include "mainDebugSubrealKV.cpp"
#include "mainDebugSubrealPEG.cpp"
#include "mainDebugWav.cpp"
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

FactoryHallway factoryHallway;

int main() {
    // skip binding of factory - no use really..
    std::cout << "Enter a number: ";
    int number;

    // comment either row there to get the selction stop or not
    std::cin >> number;
    // number = 2;

    if (!std::cin) {
        std::cerr << "Invalid input!" << std::endl;
    } else {
        switch (number) {
        case 1:
            debugSubreal();
            break;
        case 2:
            debugWav();
            break;
        case 3:
            debugBeatnik();
            break;
        case 4:
            debugSubrealKV();
            break;
        case 5:
            debugSubrealPEG();
            break;
        case 6:
            debugPatchLoad();
            break;
        default:
            std::cout << "illegal option" << std::endl;
        }
    }

    return 0;
}
