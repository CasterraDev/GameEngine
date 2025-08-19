#pragma once

#include "defines.h"

typedef struct SystemsInfo {
    u64 systemMemReqPlatform;
    void* systemMemBlockPlatform;

    u64 systemMemReqEvent;
    void* systemMemBlockEvent;

    u64 systemMemReqLogging;
    void* systemMemBlockLogging;

    u64 systemMemReqInput;
    void* systemMemBlockInput;
} SystemsInfo;

b8 systemsInit(SystemsInfo* si);

b8 systemsShutdown(SystemsInfo* si);
