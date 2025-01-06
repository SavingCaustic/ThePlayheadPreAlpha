#pragma once
#include <iostream>
#include <string>

// I THINK THIS CLASS SHOULD GO. Artifact from problems with CD between PE and rack.

class PlayerEngine; // Forward declaration

class ErrorWriter {
  public:
    // Constructor takes a reference to PlayerEngine (dependency injection)
    explicit ErrorWriter(PlayerEngine &playerEngine) : playerEngine_(playerEngine) {
        std::cout << "setting up error-writer" << std::endl;
    }

    // Log an error by calling the PlayerEngine's method to enqueue the error
    void logError(int code, const std::string &message);

  private:
    PlayerEngine &playerEngine_; // Reference to PlayerEngine instance
};