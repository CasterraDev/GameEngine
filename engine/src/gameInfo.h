#pragma once
#include "defines.h"

typedef struct GameInfo {
    char* appName;
    i32 x;
    i32 y;
    i32 width;
    i32 height;

    b8 (*init)(struct GameInfo* game_inst);

    b8 (*update)(struct GameInfo* game_inst, f32 delta_time);

    b8 (*render)(struct GameInfo* game_inst, f32 delta_time);

    // Any state that the game may need
    void* state;
} GameInfo;
