#include "dinoarray.h"
#include "core/systems/fmemory.h"
#include <stdio.h>
#include <string.h>

// TODO: Fix this malloc/free thing
#ifndef DINO_MALLOC
#define DINO_MALLOC(size) fmalloc(size, MEMORY_TAG_DINO)
#endif

#ifndef DINO_FREE
#define DINO_FREE(block, size) ffree(block, size, MEMORY_TAG_DINO)
#endif

void* _dino_create(unsigned long long length, unsigned long long stride,
                   _Bool setLength) {
    // Like an html network header
    // Stores info
    unsigned long long header =
        DINOARRAY_FIELD_LENGTH * sizeof(unsigned long long);
    unsigned long long mix = (length * stride) + header;
    void* newArr = DINO_MALLOC(mix);
    // Set header info
    ((unsigned long long*)newArr)[DINOARRAY_MAX_SIZE] = length;
    ((unsigned long long*)newArr)[DINOARRAY_LENGTH] = (setLength) ? length : 0;
    ((unsigned long long*)newArr)[DINOARRAY_STRIDE] = stride;
    // Move the array up so the user can access their elements immediately
    return ((void*)(((unsigned long long*)newArr) + DINOARRAY_FIELD_LENGTH));
}

void _dino_destroy(void* array) {
    unsigned long long* header =
        (unsigned long long*)array - DINOARRAY_FIELD_LENGTH;
    DINO_FREE(header,
              (dinoLength(array) * dinoStride(array)) +
                  (sizeof(unsigned long long) * DINOARRAY_FIELD_LENGTH));
}

void* _dino_resize(void* array) {
    unsigned long long length = dinoLength(array);
    unsigned long long stride = dinoStride(array);
    void* temp = _dino_create(dinoMaxSize(array) * DINO_DEFAULT_RESIZE_FACTOR,
                              stride, false);
    memcpy(temp, array, length * stride);
    _dino_destroy(array);
    dinoLengthSet(temp, length);
    return temp;
}

void* _dino_shrink(void* array) {
    unsigned long long length = dinoLength(array);
    unsigned long long stride = dinoStride(array);
    void* temp = _dino_create(length + 1, stride, false);
    memcpy(temp, array, length * stride);
    _dino_destroy(array);
    dinoLengthSet(temp, length);
    return temp;
}

unsigned long long _dino_field_get(void* array, unsigned long long field) {
    unsigned long long* header =
        (unsigned long long*)array - DINOARRAY_FIELD_LENGTH;
    return (header[field]);
}

void _dino_field_set(void* array, unsigned long long field,
                     unsigned long long value) {
    unsigned long long* header =
        (unsigned long long*)array - DINOARRAY_FIELD_LENGTH;
    header[field] = value;
}

void* _dino_push(void* array, const void* valuePtr) {
    unsigned long long length = dinoLength(array);
    unsigned long long stride = dinoStride(array);
    // printf("Length: %llu, MaxSize: %llu\n", length, dinoMaxSize(array));
    if (length >= dinoMaxSize(array)) {
        // printf("Resizing Array\n");
        array = _dino_resize(array);
    }
    unsigned long long idx = (unsigned long long)array;
    // Since length is One-based and array is Zero-based we don't have to add
    // one for the new element
    idx += length * stride;
    memcpy((void*)idx, valuePtr, stride);
    dinoLengthSet(array, length + 1);
    return array;
}

void* _dino_insert_at(void* array, unsigned long long idx, void* valuePtr) {
    unsigned long long length = dinoLength(array);
    unsigned long long stride = dinoStride(array);
    if (idx >= length) {
        fprintf(stderr, "DINO ERROR: Index was more than array length");
        return array;
    }
    if (length >= dinoMaxSize(array)) {
        array = _dino_resize(array);
    }
    unsigned long long memIdx = (unsigned long long)array;

    // If idx isn't at the end move elements after it down one
    // Element after the index to move the afterbit to
    unsigned long long elementAfter = memIdx + ((idx + 1) * stride);
    unsigned long long afterbit = memIdx + (idx * stride);
    if (idx != length - 1) {
        memcpy((void*)elementAfter, (void*)afterbit, stride * (length - idx));
    }
    // Actually copy the idx value into the array
    memcpy((void*)(memIdx + (idx * stride)), valuePtr, stride);
    dinoLengthSet(array, length + 1);
    return array;
}

void _dino_pop(void* array, void* dest) {
    unsigned long long length = dinoLength(array);
    unsigned long long stride = dinoStride(array);
    unsigned long long idx = (unsigned long long)array;
    idx += (length - 1) * stride;
    memcpy(dest, (void*)idx, stride);
    dinoLengthSet(array, length - 1);
}

void* _dino_pop_at(void* array, unsigned long long idx, void* dest) {
    unsigned long long length = dinoLength(array);
    if (idx >= length) {
        fprintf(stderr, "DINO ERROR: Index was more than array length");
        return array;
    }
    unsigned long long stride = dinoStride(array);
    unsigned long long memIdx = (unsigned long long)array;
    unsigned long long eleIdx = idx * stride;
    // Copy the element to the dest
    memcpy(dest, (void*)(memIdx + eleIdx), stride);

    // If idx isn't at the end move elements after it down one
    // Element after the index to move the afterbit to
    unsigned long long elementAfter = memIdx + ((idx + 1) * stride);
    unsigned long long afterbit = memIdx + (idx * stride);
    if (idx != length - 1) {
        memcpy((void*)afterbit, (void*)elementAfter, stride * (length - idx));
    }
    dinoLengthSet(array, length - 1);
    return array;
}
