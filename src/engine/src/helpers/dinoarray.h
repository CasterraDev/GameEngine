#pragma once

/**
 *  NOTE: If you want to pass a Dino array to a function. Be sure to pass it as
 * an [type]** because [type]* is the dino array; The extra * makes it a
 * "pointer" or use the types defined below to make it easier to read
 */

enum {
    DINOARRAY_MAX_SIZE,
    DINOARRAY_LENGTH,
    DINOARRAY_STRIDE,
    DINOARRAY_FIELD_LENGTH
};

// Types
#define DinoString char**
#define DinoChar char*
#define DinoInt signed int*
#define DinoDouble signed double*
#define DinoFloat signed float*
#define DinoLong signed long*
#define DinoLongLong signed long long*

#define DinoIntU unsigned int*
#define DinoDoubleU unsigned double*
#define DinoFloatU unsigned float*
#define DinoLongU unsigned long*
#define DinoLongLongU unsigned long long*

//"Private Functions" will wrap with define functions

void* _dino_create(unsigned long long length, unsigned long long stride, _Bool setLength);
void _dino_destroy(void* array);
void* _dino_resize(void* array);
void* _dino_shrink(void* array);

unsigned long long _dino_field_get(void* array, unsigned long long field);
void _dino_field_set(void* array, unsigned long long field,
                     unsigned long long value);

void* _dino_push(void* array, const void* valuePtr);
void _dino_pop(void* array, void* dest);

void* _dino_pop_at(void* array, unsigned long long idx, void* dest);
void* _dino_insert_at(void* array, unsigned long long idx, void* valuePtr);

#define DINO_DEFAULT_SIZE 1
#define DINO_DEFAULT_RESIZE_FACTOR 2

//====================== Define function wrappers ======================

/**
 *  Create a Dino array. Initial size will be 1. For custom size look at
 * `dinoCreateReserve()`
 */
#define dinoCreate(type) _dino_create(DINO_DEFAULT_SIZE, sizeof(type), false);

/**
 *  Create a Dino array with a custom initial length.
 *  So if you know you need 32 elements you should do dinoCreateReserve(32,
 * [type]). This saves on having to reDINO_MALLOC a ton of times.
 * Note: This doesn't set the length. Length is only changed when dinoPush/dinoPop (an similar FNs) are called.
 * If you want the length set look at `dinoCreateReserveWithLengthSet` or `dinoLengthSet`
 */
#define dinoCreateReserve(length, type) _dino_create(length, sizeof(type), false);

/**
 *  Create a Dino array with a custom initial length.
 *  Same as `dinoCreateReserve` except it automatically sets length to the max length.
 *  This is good if you need an dynamic array for a hashmap where any spot may be filled/empty and is not filled linearly
 *  NOTE: Should not use `dinoPush`/`dinoPop`, it will still effect the length
 */
#define dinoCreateReserveWithLengthSet(length, type) _dino_create(length, sizeof(type), true);

/**
 *  Frees the Dino array
 */
#define dinoDestroy(array) _dino_destroy(array);

/**
 *  Shrinks the Dino array to it's length so no memory is being wasted.
 *  This does perform a reallocate.
 */
#define dinoShrink(array) _dino_shrink(array);

/**
 *  Push an element value to the Dino array. Will automatically resize
 */
#define dinoPush(array, value)                                                 \
    {                                                                          \
        typeof(value) t = value;                                               \
        array = _dino_push(array, &t);                                         \
    }

/**
 *  Pop the last element value from the Dino array.
 */
#define dinoPop(array, valuePtr) _dino_pop(array, valuePtr)

/**
 *  Insert an element value to the Dino array at a certain index. Will
 * automatically resize
 */
#define dinoInsertAt(array, index, value)                                      \
    {                                                                          \
        typeof(value) temp = value;                                            \
        array = _dino_insert_at(array, index, &temp);                          \
    }

/**
 *  Pop the element value with the index from the Dino array.
 */
#define dinoPopAt(array, index, value_ptr) _dino_pop_at(array, index, value_ptr)

//====================== QOL Functions Defined ======================

/**
 *  Clears the Dino array.
 *  Does NOT do any reallocating. So the memory will still be DINO_MALLOCed.
 */
#define dinoClear(array) _dino_field_set(array, DINOARRAY_LENGTH, 0)

/**
 *  Sets the length of the Dino Array.
 *  Made to be a helper function for other DinoArray functions. Users be
 * careful.
 */
#define dinoLengthSet(array, value)                                            \
    _dino_field_set(array, DINOARRAY_LENGTH, value)

//====================== QOL Variables Defined ======================

/**
 *  Get the Max Size / Capacity of the Dino Array
 */
#define dinoMaxSize(array) _dino_field_get(array, DINOARRAY_MAX_SIZE)

/**
 *  Get the Length of the Dino Array
 */
#define dinoLength(array) _dino_field_get(array, DINOARRAY_LENGTH)

/**
 *  Get the Stride of the Dino Array
 */
#define dinoStride(array) _dino_field_get(array, DINOARRAY_STRIDE)

