#pragma once

#include "defines.h"
// Make sure MEMORY_TAG_MAX_TAGS is ALWAYS at the end. It's used as a sort of
// null pointer for loops
#define FOREACH_TAG(TAG)                                                       \
    TAG(MEMORY_TAG_UNKNOWN)                                                    \
    TAG(MEMORY_TAG_ALLOCATORS)                                                 \
    TAG(MEMORY_TAG_APPLICATION)                                                \
    TAG(MEMORY_TAG_ARRAY)                                                      \
    TAG(MEMORY_TAG_DINO)                                                       \
    TAG(MEMORY_TAG_GAME)                                                       \
    TAG(MEMORY_TAG_RENDERER)                                                   \
    TAG(MEMORY_TAG_RESOURCE)                                                   \
    TAG(MEMORY_TAG_TEXTURE)                                                    \
    TAG(MEMORY_TAG_STRING)                                                     \
    TAG(MEMORY_TAG_SYSTEM)                                                     \
    TAG(MEMORY_TAG_MAX_TAGS)

#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,

typedef enum MemoryTag { FOREACH_TAG(GENERATE_ENUM) } MemoryTag;

static const char* TAG_STRING[] = {FOREACH_TAG(GENERATE_STRING)};

typedef struct MemorySystemSettings {
    u64 totalSize;
} MemorySystemSettings;

/**
 * @brief Sets up the memory system. This system will be used to perform most
 * application memory allocations. Does NOT need to be called twice (Unlike most
 * systems).
 * @param settings The settings needed to set up the memory system
 * @returns true if successful, false if failed
 */
CT_API b8 memoryInit(MemorySystemSettings settings);

/**
 * @brief Shuts down the memory system. Performs a memory free.
 */
CT_API void memoryShutdown();

/**
 * @brief Allocates memory. (Doesn't actually perform a malloc);
 * @param size Size of the block of memory needed
 * @param tag Memory tag used for debugging purposes to see memory leaks
 * @returns pointer to a block of memory, 0 if failed and outputs an error
 * message
 */
CT_API void* fmalloc(u64 size, MemoryTag tag);

/**
 * @brief Frees a block of memory
 * @param block Pointer to the memory block
 * @param size Size of the block of memory needed to be freed
 * @param tag Memory tag used for debugging purposes to see memory leaks
 * @returns pointer to a block of memory, 0 if failed and outputs an error
 * message
 */
CT_API void ffree(void* block, u64 size, MemoryTag tag);

/**
 * @brief Zeros out a block of memory
 * @param block Pointer to the memory block
 * @param size Size of the block of memory
 * @returns pointer to a block of memory
 */
CT_API void* fzeroMemory(void* block, u64 size);

/**
 * @brief Copies a block of memory to another block
 * @param dest Pointer to the memory block of the destination
 * @param source Pointer to the memory block of the source which will be copied
 * @param size Size of the amount of memory you want to copy
 * @returns pointer to a block of memory
 */
CT_API void* fcpyMem(void* dest, const void* source, u64 size);

/**
 * @brief Sets a block of memory to `value`
 * @param dest Pointer to the memory block of the destination
 * @param value The value you want to set the memory to
 * @param size Size of the amount of memory to set
 * @returns pointer to a block of memory
 */
CT_API void* fsetMem(void* dest, i32 value, u64 size);

/**
 * @brief Prints the Memory Tags for debugging purposes
 */
CT_API void printMemoryUsage();
