#pragma once

//==================TYPEDEFS=====================

// Unsigned int types.
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

// Signed int types.
typedef signed char i8;
typedef signed short i16;
typedef signed int i32;
typedef signed long long i64;

// Boolean types
typedef int b32;
typedef char b8;

// Floating point types
typedef float f32;
typedef double f64;

//==================DEFINES=====================

#define true 1
#define false 0

//==================Invalid IDs=================
// Last valid number within the type's range
#define INVALID_ID 4294967295U
#define INVALID_ID_U16 65535U
#define INVALID_ID_U8 255U

// Abstract static_assert becuz clang & gcc like to be special
#if defined(__clang__) || defined(__gcc__)
#define STATIC_ASSERT _Static_assert
#else
#define STATIC_ASSERT static_assert
#endif

// Make sure that the types are the sizes that they should be.
// Some platforms are different
STATIC_ASSERT(sizeof(i8) == 1, "Expected i8 to be 1 byte.");
STATIC_ASSERT(sizeof(i16) == 2, "Expected i16 to be 2 bytes.");
STATIC_ASSERT(sizeof(i32) == 4, "Expected i32 to be 4 bytes.");
STATIC_ASSERT(sizeof(i64) == 8, "Expected i64 to be 8 bytes.");

STATIC_ASSERT(sizeof(u8) == 1, "Expected u8 to be 1 byte.");
STATIC_ASSERT(sizeof(u16) == 2, "Expected u16 to be 2 bytes.");
STATIC_ASSERT(sizeof(u32) == 4, "Expected u32 to be 4 bytes.");
STATIC_ASSERT(sizeof(u64) == 8, "Expected u64 to be 8 bytes.");

STATIC_ASSERT(sizeof(f32) == 4, "Expected f32 to be 4 bytes.");
STATIC_ASSERT(sizeof(f64) == 8, "Expected f64 to be 8 bytes.");

//==================Space Unit FNs==============
#define GIGABYTES(amount) amount * 1000000000
#define MEGABYTES(amount) amount * 1000000
#define KILOBYTES(amount) amount * 1000

#define GIBIBYTES(amount) amount * 1024 * 1024 * 1024
#define MEBIBYTES(amount) amount * 1024 * 1024
#define KIBIBYTES(amount) amount * 1024

#ifdef GE_EXPORT
// Exports
#ifdef _MSC_VER
#define CT_API __declspec(dllexport)
#else
#define CT_API __attribute__((visibility("default")))
#endif
#else
// Imports
#ifdef _MSC_VER
#define CT_API __declspec(dllimport)
#else
#define CT_API
#endif
#endif

// Inlining
#ifdef _MSC_VER
#define GE_INLINE __forceinline
#define GE_NOINLINE __declspec(noinline)
#else
#define GE_INLINE static inline
#define GE_NOINLINE
#endif

// Platform detection
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#define GE_PLATFORM_WINDOWS 1
#ifndef _WIN64
#error "64-bit is required on Windows!"
#endif
#elif defined(__linux__) || defined(__gnu_linux__)
// Linux OS
#define GE_PLATFORM_LINUX 1
#if defined(__ANDROID__)
#define GE_PLATFORM_ANDROID 1
#endif
#elif defined(__unix__)
// Catch anything not caught by the above.
#define GE_PLATFORM_UNIX 1
#elif defined(_POSIX_VERSION)
// Posix
#define GE_PLATFORM_POSIX 1
#elif __APPLE__
// Apple platforms
#define GE_PLATFORM_APPLE 1
#include <TargetConditionals.h>
#if TARGET_IPHONE_SIMULATOR
// iOS Simulator
#define GE_PLATFORM_IOS 1
#define GE_PLATFORM_IOS_SIMULATOR 1
#elif TARGET_OS_IPHONE
#define GE_PLATFORM_IOS 1
// iOS device
#elif TARGET_OS_MAC
// Other kinds of Mac OS
#else
#error "Unknown Apple platform"
#endif
#else
#error "Unknown platform!"
#endif
