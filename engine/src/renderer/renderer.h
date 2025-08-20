#include "defines.h"
#include "renderer/renderInfo.h"

typedef enum RendererType { RENDERER_TYPE_VULKAN } RendererType;

b8 rendererInit(u64* memoryRequirement, void* state, RendererType type);
void rendererShutdown(void* state);
void rendererResized(u16 width, u16 height);
b8 rendererDraw(RenderPacket* packet);
