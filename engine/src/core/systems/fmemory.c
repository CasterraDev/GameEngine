#include "fmemory.h"

#include "core/dynamicAllocator.h"
#include "core/systems/logger.h"
#include "platform/platform.h"

// TODO: Custom string lib
#include <stdio.h>

/*
 *  Currently using a dynamic allocator to allocate all memory at the start of
 * the program then it handles sectioning blocks and freeing them when needed.
 */

typedef struct memoryStats {
    u64 totalMemAllocced;
    u64 totalMemAllocsByTag[MEMORY_TAG_MAX_TAGS];
} memoryStats;

typedef struct memorySystemState {
    // The stats for the memory. Used more for engine development. Might keep it
    // who knows
    memoryStats stats;
    // The settings for the memory system for more flexibity. (e.g. use
    // dynamicAllocator or not)
    MemorySystemSettings settings;
    // The memory requirement needed for the dynamicAllocator to work. (e.g.
    // Gamedev asked for 1Gib, so total memory needed could be 1.2Gib or
    // something)
    u64 allocatorMemReq;
    // Block of memory the dynamicAllocator is allocated at
    void* allocatorBlock;
    // Ref to the dynamicAllocator
    dynaAllocator allocator;
} memorySystemState;

static memorySystemState* systemPtr;

b8 memoryInit(MemorySystemSettings settings) {
    u64 stateMemReq = sizeof(memorySystemState);
    // Get the total memory required now
    u64 dynaMemReq = 0;
    dynaAllocCreate(settings.totalSize, &dynaMemReq, 0, 0);

    // Since this is the memory system it can allocate it's own memory
    void* block = platformAllocate(stateMemReq + dynaMemReq, false);
    if (!block) {
        FFATAL("Failed to allocate memory.");
        return false;
    }

    systemPtr = (memorySystemState*)block;
    systemPtr->settings = settings;
    systemPtr->allocatorMemReq = dynaMemReq;

    // Zero out stats
    platformZeroMemory(&systemPtr->stats, sizeof(systemPtr->stats));

    systemPtr->allocatorBlock = ((void*)block + stateMemReq);

    // Actually create the dynamicAllocator
    if (!dynaAllocCreate(settings.totalSize, &systemPtr->allocatorMemReq,
                         systemPtr->allocatorBlock, &systemPtr->allocator)) {
        FFATAL("MemoryInit Failed to allocate a dynamicAllocator.");
        return false;
    }

    FDEBUG("Memory System allocated %llu bytes", settings.totalSize);
    return true;
}

void memoryShutdown() {
    if (!systemPtr) {
        FWARN("`memoryShutdown` called before `memoryInit`");
        return;
    }
    dynaAllocDestroy(&systemPtr->allocator);
    platformFree(systemPtr,
                 systemPtr->allocatorMemReq + sizeof(memorySystemState));
    systemPtr = 0;
}

void* fmalloc(u64 size, MemoryTag tag) {
    // UNKNOWN can be used but probably shouldn't be
    if (tag == MEMORY_TAG_UNKNOWN) {
        FWARN("fallocate called using MEMORY_TAG_UNKNOWN.");
    }

    void* block = 0;

    if (systemPtr) {
        systemPtr->stats.totalMemAllocced += size;
        systemPtr->stats.totalMemAllocsByTag[tag] += size;

        block = dynaAlloc(&systemPtr->allocator, size);
    }

    // As a fallback incase the dynamicAllocator fails which it should never
    // do
    if (!block) {
        FWARN("fmalloc called before memory system was inited or memory system "
              "failed.");
        block = platformAllocate(size, false);
    }

    // Zero out the memory so no old junk will confuse the user
    platformZeroMemory(block, size);
    return block;
}

void ffree(void* block, u64 size, MemoryTag tag) {
    // UNKNOWN can be used but probably shouldn't be
    if (tag == MEMORY_TAG_UNKNOWN) {
        FWARN("ffree called using MEMORY_TAG_UNKNOWN.");
    }

    if (systemPtr) {
        systemPtr->stats.totalMemAllocced -= size;
        systemPtr->stats.totalMemAllocsByTag[tag] -= size;

        b8 result = dynaAllocFree(&systemPtr->allocator, size, block);

        // If result is false then that means something was allocated before the
        // memory system was inited. Try to free it from the platform.
        if (!result) {
            // TODO: Memory alignment
            platformFree(block, false);
        }
    } else {
        // TODO: Memory alignment
        platformFree(block, false);
    }
}

void* fzeroMemory(void* block, u64 size) {
    return platformZeroMemory(block, size);
}

void* fcpyMem(void* dest, const void* source, u64 size) {
    return platformCopyMemory(dest, source, size);
}

void* fsetMem(void* dest, i32 value, u64 size) {
    return platformSetMemory(dest, value, size);
}

void printMemoryUsage() {
    if (!systemPtr) {
        FERROR("printMemoryUsage: Memory system has not been inited.")
        return;
    }
    const u64 gib = 1073741824; // 1024 * 1024 * 1024
    const u64 mib = 1048576;    // 1024 * 1024
    const u64 kib = 1024;

    for (u32 i = 0; i < MEMORY_TAG_MAX_TAGS; ++i) {
        char unit[4] = "XiB";
        float amount = 1.0f;
        if (systemPtr->stats.totalMemAllocsByTag[i] >= gib) {
            unit[0] = 'G';
            amount = systemPtr->stats.totalMemAllocsByTag[i] / (float)gib;
        } else if (systemPtr->stats.totalMemAllocsByTag[i] >= mib) {
            unit[0] = 'M';
            amount = systemPtr->stats.totalMemAllocsByTag[i] / (float)mib;
        } else if (systemPtr->stats.totalMemAllocsByTag[i] >= kib) {
            unit[0] = 'K';
            amount = systemPtr->stats.totalMemAllocsByTag[i] / (float)kib;
        } else {
            unit[0] = 'B';
            unit[1] = 0;
            amount = (float)systemPtr->stats.totalMemAllocsByTag[i];
        }

        printf("  %-20s: %.3f%s\n", TAG_STRING[i], amount, unit);
    }
}
