#pragma once

#include "defines.h"

typedef enum RendererType { RENDERER_TYPE_VULKAN } RendererType;

typedef struct RenderPacket {
    b8 (*rendererInit)();
    b8 (*rendererDestroy)();
    b8 (*rendererDraw)();

    // Custom state for the renderer to put things. E.G. Vulkan Rendpass vars, OpenGL Colors
    void* rendererState;
} RenderPacket;
