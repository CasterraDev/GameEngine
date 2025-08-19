#include "engine.h"
#include "core/systems/fmemory.h"
#include "core/systems/logger.h"
#include "core/systemsManager.h"
#include "defines.h"
#include "gameInfo.h"
#include "platform/platform.h"

typedef struct EngineInfo {
    GameInfo* gameInfo;
    b8 isRunning;
    SystemsInfo systemsInfo;
} EngineInfo;

static EngineInfo* systemPtr;

b8 engineStart(GameInfo* gameInfo) {
    FINFO("Started Engine.");
    systemPtr = fmalloc(sizeof(EngineInfo), MEMORY_TAG_APPLICATION);

    MemorySystemSettings memorySettings;
    memorySettings.totalSize = GIBIBYTES(1);
    memoryInit(memorySettings);

    systemsInit(&systemPtr->systemsInfo);

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
    systemsShutdown(&systemPtr->systemsInfo);
    return true;
}
