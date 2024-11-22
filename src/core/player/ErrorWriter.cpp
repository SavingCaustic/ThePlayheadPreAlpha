// ErrorWriter.cpp
#include "ErrorWriter.h"
#include "PlayerEngine.h"

void ErrorWriter::logError(int code, const std::string &message) {
    playerEngine_.sendError(code, message); // Delegate the error to PlayerEngine
}
