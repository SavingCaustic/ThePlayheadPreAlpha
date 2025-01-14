#include "mainDebugSubreal.cpp"
#include "mainDebugSubrealKV.cpp"
#include "mainDebugSubrealPEG.cpp"
#include "mainDebugWav.cpp"
#include "mainDebugWavLoad.cpp"
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

int main() {
    std::cout << "Enter a number: ";
    int number;
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
            debugWavLoad();
            break;
        case 4:
            debugSubrealKV();
            break;
        case 5:
            debugSubrealPEG();
            break;
        default:
            std::cout << "illegal option" << std::endl;
        }
    }

    return 0;
}
