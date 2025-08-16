#pragma once

#include "defines.h"

typedef struct freelist {
    void* memory;
    u64 memorySize;
} freelist;

/*
 * Since the memory is malloced externally. There is no destroy FN. Instead use
 * `freelistClear` then free the memory yourself
 */
CT_API void freelistCreate(u64 totalSize, u64* memoryReq, void* memory,
                           freelist* outList);

CT_API b8 freelistAllocateBlock(freelist* list, u64 size, u64* outOffset);

CT_API b8 freelistFreeBlock(freelist* list, u64 size, u64 offset);

/*
 * `outOldMemory` must be freed externally
 */
CT_API b8 freelistResize(freelist* list, u64* memoryReq, u64 size,
                         void* newMemory, void* outOldMemory);

CT_API void freelistClear(freelist* list);

CT_API u64 freelistFreeSpace(freelist* list);

/*
 * Still need to free the memory yourself
 */
#define freelistDestroy(list) freelistClear(list);
