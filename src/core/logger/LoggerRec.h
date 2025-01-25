#pragma once

// no namespace for error..

constexpr int LOG_DEBUG = 1;
constexpr int LOG_INFO = 2;
constexpr int LOG_WARNING = 4;
constexpr int LOG_ERROR = 8;
constexpr int LOG_CRITICAL = 16;

struct LoggerRec {
    int code; // BIT MASKED, not http status code ..
    char message[100];
};