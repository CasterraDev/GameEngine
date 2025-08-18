#include "logger.h"
#include "platform/filesystem.h"
#include "platform/platform.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

typedef struct loggerState {
    char* logQueue;
    u64 fileLogQueueCnt;
    FileHandle fileHandle;
} loggerState;

static loggerState* systemPtr;

void sendTextToFile(const char* m) {
    // TODO: Make this use a queue
    u64 l = strlen(m);
    u64 written = 0;
    if (!fsWrite(&systemPtr->fileHandle, l, m, &written)) {
        FERROR("Failed to write to log file");
    }
}

b8 loggerInit(u64* memoryRequirement, void* state) {
    *memoryRequirement = sizeof(loggerState);
    if (state == 0) {
        return true;
    }
    systemPtr = state;

    if (!fsOpen("appLogger.log", FILE_MODE_WRITE, false,
                &systemPtr->fileHandle)) {
        FERROR("Couldn't open appLogger.log to write logs.");
        return false;
    }
    return true;
}

void loggerShutdown() {
    systemPtr = 0;
    // TODO: cleanup logging/write queued entries.
}

void logToFile(logLevel level, b8 logToConsole, const char* message, ...) {
    const char* levelStr[6] = {"[FATAL]: ", "[ERROR]: ", "[WARN]:  ",
                               "[INFO]:  ", "[DEBUG]: ", "[TRACE]: "};
    char outMessage[28000];
    memset(outMessage, 0, sizeof(outMessage));

    __builtin_va_list arg_ptr;
    va_start(arg_ptr, message);
    // Note: Should be the same as `outMessage`
    vsnprintf(outMessage, 28000, message, arg_ptr);
    va_end(arg_ptr);

    // Note: Gives a 28k limit for log messages. Should be the same as
    // `outMessage`
    char finalMessage[28000];
    sprintf(finalMessage, "%s%s\n", levelStr[level], outMessage);

    if (logToConsole) {
        if (level < 2) {
            platformConsoleWriteError(finalMessage, level);
        } else {
            platformConsoleWrite(finalMessage, level);
        }
    }

    sendTextToFile(finalMessage);
}

void logOutput(logLevel level, const char* message, ...) {
    const char* levelStr[6] = {"[FATAL]: ", "[ERROR]: ", "[WARN]:  ",
                               "[INFO]:  ", "[DEBUG]: ", "[TRACE]: "};
    char outMessage[28000];
    memset(outMessage, 0, sizeof(outMessage));

    __builtin_va_list arg_ptr;
    va_start(arg_ptr, message);
    // Note: Should be the same as `outMessage`
    vsnprintf(outMessage, 28000, message, arg_ptr);
    va_end(arg_ptr);

    // Note: Gives a 28k limit for log messages. Should be the same as
    // `outMessage`
    char finalMessage[28000];
    sprintf(finalMessage, "%s%s\n", levelStr[level], outMessage);

    if (level < 2) {
        platformConsoleWrite(finalMessage, level);
    } else {
        platformConsoleWrite(finalMessage, level);
    }
}

void reportAssertFailure(const char* expression, const char* message,
                         const char* file, i32 line) {
    logOutput(LOG_LEVEL_FATAL,
              "Assertion Failure: %s, message: '%s', in file: %s, line: %d\n",
              expression, message, file, line);
}
