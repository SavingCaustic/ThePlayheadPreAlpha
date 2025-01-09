#include "mainDebugSubreal.cpp"
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
    // std::cin >> number;
    number = 1;

    if (!std::cin) {
        std::cerr << "Invalid input!" << std::endl;
    } else {
        switch (number) {
        case 1:
            debugSubreal();
            break;
        default:
            std::cout << "illegal option" << std::endl;
        }
    }

    return 0;
}
