#pragma once

#include "defines.h"

typedef struct freelist {
    void* memory;
} freelist;

CT_API void freelistCreate(u64 totalSize, u64* memoryReq, void* memory,
                           freelist* outList);

CT_API void freelistDestroy(freelist* list);

CT_API b8 freelistAllocateBlock(freelist* list, u64 size, u64* outOffset);

CT_API b8 freelistFreeBlock(freelist* list, u64 size, u64 offset);

CT_API b8 freelistResize(freelist* list, u64* memoryReq, u64 size, void* newMemory, void* oldMemory);

CT_API void freelistClear(freelist* list);

CT_API u64 freelistFreeSpace(freelist* list);
