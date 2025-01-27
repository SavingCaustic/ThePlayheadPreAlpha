#include <stdarg.h>
#include <stdio.h>

#define LOG_MSG_LEN 100 // Match LoggerRec's message size

#define FORMAT_LOG_MESSAGE(logger, level, fmt, ...)                  \
    do {                                                             \
        (logger).code = (level);                                     \
        snprintf((logger).message, LOG_MSG_LEN, (fmt), __VA_ARGS__); \
    } while (0)

constexpr int LOG_DEBUG = 1;
constexpr int LOG_INFO = 2;
constexpr int LOG_WARNING = 4;
constexpr int LOG_ERROR = 8;
constexpr int LOG_CRITICAL = 16;

struct LoggerRec {
    int code; // BIT MASKED, not HTTP status code
    char message[LOG_MSG_LEN];
};

int main(void) {
    LoggerRec test;

    // Format the log message using the macro
    FORMAT_LOG_MESSAGE(test, LOG_CRITICAL, "Something bad happened with %s at %.2f", "someCharArr", 3.141592654f);

    // Print the prepared log message
    printf("Log Level: %d, Message: %s\n", test.code, test.message);

    return 0;
}
