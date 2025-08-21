#include "renderer.h"
#include "core/systems/logger.h"
#include "renderer/renderInfo.h"
#include "renderer/vulkan/vulkan.h"

typedef struct RendererSystemState {
    RenderPacket packet;
    RendererType type;
} RendererSystemState;

static RendererSystemState* systemPtr;

b8 setFNs(RendererType type) {
    switch (type) {
        case RENDERER_TYPE_VULKAN:
            systemPtr->packet.rendererInit = vulkanInit;
            systemPtr->packet.rendererDestroy = vulkanDestroy;
            systemPtr->packet.rendererDraw = vulkanDraw;
            break;
    }
    return true;
}

b8 rendererInit(u64* memoryRequirement, void* state, RendererType type) {
    *memoryRequirement = sizeof(RendererSystemState);
    if (state == 0){
        return true;
    }
    
    systemPtr = state;
    systemPtr->type = type;

    setFNs(type);

    FINFO("Starting Renderer");
    if (!systemPtr->packet.rendererInit()){
        FWARN("Renderer failed to Init");
        return false;
    }

    return true;
}

void rendererShutdown(void* state) {
    if (!systemPtr->packet.rendererDestroy()){
        FWARN("Renderer failed to Init");
        return;
    }
}
