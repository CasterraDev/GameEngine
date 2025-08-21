#include "systemsManager.h"
#include "core/systems/event.h"
#include "core/systems/fmemory.h"
#include "core/systems/input.h"
#include "core/systems/logger.h"
#include "platform/platform.h"
#include "renderer/renderer.h"
#include "renderer/renderInfo.h"

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

    rendererInit(&si->systemMemReqRenderer, 0, RENDERER_TYPE_VULKAN);
    si->systemMemBlockRenderer =
        fmalloc(si->systemMemReqRenderer, MEMORY_TAG_SYSTEM);
    rendererInit(&si->systemMemReqRenderer, si->systemMemBlockRenderer, RENDERER_TYPE_VULKAN);

    return true;
}

b8 systemsShutdown(SystemsInfo* si) {
    FINFO("Starting Engine Shutdown");
    rendererShutdown(si->systemMemBlockRenderer);
    platformShutdown();
    inputShutdown(si->systemMemBlockInput);
    loggerShutdown();
    eventShutdown();
    return true;
}
