#pragma once

#include "defines.h"

#define GE_ASSERTIONS_ENABLED

#ifdef GE_ASSERTIONS_ENABLED
#if _MSC_VER
#include <intrin.h>
#define debugBreak() __debugbreak()
#else
#define debugBreak() __builtin_trap()
#endif

CT_API void reportAssertFailure(const char* expression, const char* message,
                                const char* file, i32 line);

#define FASSERT(expr)                                                          \
    {                                                                          \
        if (expr) {                                                            \
        } else {                                                               \
            reportAssertFailure(#expr, "", __FILE__, __LINE__);                \
            debugBreak();                                                      \
        }                                                                      \
    }

#define FASSERT_MSG(expr, message)                                             \
    {                                                                          \
        if (expr) {                                                            \
        } else {                                                               \
            reportAssertFailure(#expr, message, __FILE__, __LINE__);           \
            debugBreak();                                                      \
        }                                                                      \
    }

#ifdef _DEBUG
#define FASSERT_DEBUG(expr)                                                    \
    {                                                                          \
        if (expr) {                                                            \
        } else {                                                               \
            reportAssertFailure(#expr, "", __FILE__, __LINE__);                \
            debugBreak();                                                      \
        }                                                                      \
    }
#else
#define FASSERT_DEBUG(expr)
#endif

#else
// Make the functions do nothing
#define FASSERT(expr)
#define FASSERT_MSG(expr, message)
#define FASSERT_DEBUG(expr)
#endif
