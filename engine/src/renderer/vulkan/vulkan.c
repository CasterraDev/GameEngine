#include "vulkan.h"
#include "core/systems/logger.h"
#include "helpers/dinoarray.h"
#include "platform/platform.h"
#include "renderer/vulkan/vulkanTypes.h"
#include "vulkan/vulkan_core.h"

// https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Base_code

static VulkanInfo vulkanInfo;

VkInstance createInstance();

b8 vulkanInit() {
    vulkanInfo.instance = createInstance();
    return true;
}

b8 vulkanDestroy() {
    return true;
}

b8 vulkanDraw() {
    return true;
}

VkInstance createInstance() {
    VkInstance vi;
    VkApplicationInfo va;
    va.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    va.pApplicationName = "Hello Triangle";
    va.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    va.pEngineName = "No Engine";
    va.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    va.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    createInfo.pApplicationInfo = &va;

    const char** dinoExtStrings = dinoCreate(const char*);
    dinoPush(dinoExtStrings, &VK_KHR_SURFACE_EXTENSION_NAME);

    platformGetRequiredExtenstions(&dinoExtStrings);

    createInfo.enabledExtensionCount = dinoLength(dinoExtStrings);
    createInfo.ppEnabledExtensionNames = dinoExtStrings;

    VkResult result = vkCreateInstance(&createInfo, 0, &vi);
    if (result != VK_SUCCESS) {
        FERROR("Failed to create instance");
    }

    dinoDestroy(dinoExtStrings);

    return vi;
}
