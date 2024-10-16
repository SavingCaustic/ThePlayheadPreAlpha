#pragma once

#include "ErrorBuffer.h"

class ErrorReader {
  public:
    // Use a reference to avoid copying
    ErrorReader(ErrorBuffer &errorBuffer) : errorBuffer(errorBuffer) {}

    void readAll() {
        // Call the correct method to read all errors
        this->errorBuffer.readAllErrors();
    }

  private:
    ErrorBuffer &errorBuffer; // Reference to the actual ErrorBuffer
};
