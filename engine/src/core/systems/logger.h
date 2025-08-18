#pragma once

#include "defines.h"

#define LOG_WARN_ENABLED 1
#define LOG_INFO_ENABLED 1
#define LOG_DEBUG_ENABLED 1
#define LOG_TRACE_ENABLED 1

// Disable debug and trace logging for release builds.
#if GE_RELEASE == 1
#define LOG_DEBUG_ENABLED 0
#define LOG_TRACE_ENABLED 0
#endif

typedef enum logLevel {
    LOG_LEVEL_FATAL = 0,
    LOG_LEVEL_ERROR = 1,
    LOG_LEVEL_WARN = 2,
    LOG_LEVEL_INFO = 3,
    LOG_LEVEL_DEBUG = 4,
    LOG_LEVEL_TRACE = 5
} logLevel;

b8 loggerInit(u64* memoryRequirement, void* state);
void loggerShutdown();

// TODO: Actually log it to the file
CT_API void logToFile(logLevel level, b8 logToConsole, const char* message,
                      ...);

CT_API void logOutput(logLevel level, const char* message, ...);

// TODO: Make file logging FNs

// Logs a fatal-level message.
#define FFATAL(message, ...) logOutput(LOG_LEVEL_FATAL, message, ##__VA_ARGS__);

#ifndef FERROR
#define FERROR(message, ...) logOutput(LOG_LEVEL_ERROR, message, ##__VA_ARGS__);
#endif

#if LOG_WARN_ENABLED == 1
#define FWARN(message, ...) logOutput(LOG_LEVEL_WARN, message, ##__VA_ARGS__);
#else
#define FWARN(message, ...)
#endif

#if LOG_INFO_ENABLED == 1
#define FINFO(message, ...) logOutput(LOG_LEVEL_INFO, message, ##__VA_ARGS__);
#else
#define FINFO(message, ...)
#endif

#if LOG_DEBUG_ENABLED == 1
#define FDEBUG(message, ...) logOutput(LOG_LEVEL_DEBUG, message, ##__VA_ARGS__);
#else
#define FDEBUG(message, ...)
#endif

#if LOG_TRACE_ENABLED == 1
#define FTRACE(message, ...) logOutput(LOG_LEVEL_TRACE, message, ##__VA_ARGS__);
#else
#define FTRACE(message, ...)
#endif
