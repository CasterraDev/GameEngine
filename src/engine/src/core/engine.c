#include "engine.h"
#include "core/fmemory.h"
#include "core/logger.h"

b8 startEngine() {
    FINFO("Started Engine.");
    printMemoryUsage();
    return true;
}
