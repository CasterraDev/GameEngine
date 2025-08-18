#include "engine.h"
#include "core/systems/event.h"
#include "core/systems/fmemory.h"
#include "core/systems/input.h"
#include "core/systems/logger.h"
#include "defines.h"
#include "gameInfo.h"
#include "platform/platform.h"

typedef struct EngineInfo {
    GameInfo* gameInfo;
    b8 isRunning;

    u64 systemMemReqPlatform;
    void* systemMemBlockPlatform;

    u64 systemMemReqEvent;
    void* systemMemBlockEvent;

    u64 systemMemReqLogging;
    void* systemMemBlockLogging;

    u64 systemMemReqInput;
    void* systemMemBlockInput;

} EngineInfo;

static EngineInfo* systemPtr;

b8 engineStart(GameInfo* gameInfo) {
    FINFO("Started Engine.");
    systemPtr = fmalloc(sizeof(EngineInfo), MEMORY_TAG_APPLICATION);

    MemorySystemSettings memorySettings;
    memorySettings.totalSize = GIBIBYTES(1);
    memoryInit(memorySettings);

    eventInit(&systemPtr->systemMemReqEvent, 0);
    systemPtr->systemMemBlockEvent =
        fmalloc(systemPtr->systemMemReqEvent, MEMORY_TAG_SYSTEM);
    eventInit(&systemPtr->systemMemReqEvent, systemPtr->systemMemBlockEvent);

    loggerInit(&systemPtr->systemMemReqLogging, 0);
    systemPtr->systemMemBlockLogging =
        fmalloc(systemPtr->systemMemReqLogging, MEMORY_TAG_SYSTEM);
    loggerInit(&systemPtr->systemMemReqLogging,
               systemPtr->systemMemBlockLogging);

    inputInit(&systemPtr->systemMemReqInput, 0);
    systemPtr->systemMemBlockInput =
        fmalloc(systemPtr->systemMemReqInput, MEMORY_TAG_SYSTEM);
    inputInit(&systemPtr->systemMemReqInput, systemPtr->systemMemBlockInput);

    // TODO: Register program events. (Resize, Buttons)

    FINFO("Starting platform")
    platformStartup(&systemPtr->systemMemReqPlatform, 0, "Triangle", 0, 0,
                    50 * 16, 50 * 9);
    systemPtr->systemMemBlockPlatform =
        fmalloc(systemPtr->systemMemReqPlatform, MEMORY_TAG_SYSTEM);
    platformStartup(&systemPtr->systemMemReqPlatform,
                    systemPtr->systemMemBlockPlatform, "Triangle", 0, 0,
                    50 * 16, 50 * 9);
    printMemoryUsage();
    systemPtr->isRunning = true;

    return true;
}

b8 engineRun(GameInfo* gameInfo) {
    while (systemPtr->isRunning) {
        if (!platformPumpMessages()) {
            systemPtr->isRunning = false;
        }
    }
    return true;
}

b8 engineDestroy(GameInfo* gameInfo) {
    platformShutdown();
    return true;
}
