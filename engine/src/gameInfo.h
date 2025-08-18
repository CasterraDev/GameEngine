#pragma once
#include "defines.h"

typedef struct GameInfo {

    b8 (*init)(struct GameInfo* game_inst);

    b8 (*update)(struct GameInfo* game_inst, f32 delta_time);

    b8 (*render)(struct GameInfo* game_inst, f32 delta_time);

    // Any state that the game may need
    void* state;
} GameInfo;
