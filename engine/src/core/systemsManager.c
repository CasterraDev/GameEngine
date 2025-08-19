#include "systemsManager.h"
#include "core/systems/event.h"
#include "core/systems/fmemory.h"
#include "core/systems/input.h"
#include "core/systems/logger.h"
#include "platform/platform.h"

b8 systemsInit(SystemsInfo* si) {
    eventInit(&si->systemMemReqEvent, 0);
    si->systemMemBlockEvent = fmalloc(si->systemMemReqEvent, MEMORY_TAG_SYSTEM);
    eventInit(&si->systemMemReqEvent, si->systemMemBlockEvent);

    loggerInit(&si->systemMemReqLogging, 0);
    si->systemMemBlockLogging =
        fmalloc(si->systemMemReqLogging, MEMORY_TAG_SYSTEM);
    loggerInit(&si->systemMemReqLogging, si->systemMemBlockLogging);

    inputInit(&si->systemMemReqInput, 0);
    si->systemMemBlockInput = fmalloc(si->systemMemReqInput, MEMORY_TAG_SYSTEM);
    inputInit(&si->systemMemReqInput, si->systemMemBlockInput);

    // TODO: Register program events. (Resize, Buttons)

    FINFO("Starting platform")
    platformInit(&si->systemMemReqPlatform, 0);
    si->systemMemBlockPlatform =
        fmalloc(si->systemMemReqPlatform, MEMORY_TAG_SYSTEM);
    platformInit(&si->systemMemReqPlatform, si->systemMemBlockPlatform);

    // Might want to move this to engine.c
    platformStartup("Triangle", 0, 0, 50 * 16, 50 * 9);
    return true;
}

b8 systemsShutdown(SystemsInfo* si) {
    platformShutdown();
    inputShutdown(si->systemMemBlockInput);
    loggerShutdown();
    eventShutdown();
    return true;
}
