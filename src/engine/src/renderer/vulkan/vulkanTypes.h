#pragma once

#include "defines.h"
#include <vulkan/vulkan_core.h>
// Checks the given expression's return value is OK.
#define VK_CHECK(expr)                                                         \
    {                                                                          \
        FASSERT(expr == VK_SUCCESS);                                           \
    }

typedef struct VulkanInfo {
    VkInstance instance;
    VkAllocationCallbacks* allocator;
    u32 deltaTime;
    VkDebugUtilsMessengerEXT debugMessenger;
    // VulkanDevice device;
    VkSurfaceKHR surface;
    // VulkanSwapchain swapchain;
    u32 framebufferWidth;
    u32 framebufferHeight;

    b8 recreatingSwapchain;
    u32 framebufferSizeGen;
    u32 framebufferSizeGenLast;
} VulkanInfo;

