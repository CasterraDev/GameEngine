#include "vulkan.h"
#include "core/systems/logger.h"
#include "helpers/dinoarray.h"
#include "platform/platform.h"
#include "renderer/vulkan/vulkanTypes.h"
#include "vulkan/vulkan_core.h"
#include <string.h>

// https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Base_code

static VulkanInfo vulkanInfo;

void createInstance();

VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT* pDebugMessenger) {
    PFN_vkCreateDebugUtilsMessengerEXT func =
        (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            instance, "vkCreateDebugUtilsMessengerEXT");
    if (func) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance,
                                   VkDebugUtilsMessengerEXT debugMessenger,
                                   const VkAllocationCallbacks* pAllocator) {
    PFN_vkDestroyDebugUtilsMessengerEXT func =
        (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != 0) {
        func(instance, debugMessenger, pAllocator);
    }
}

static VKAPI_ATTR VkBool32 VKAPI_CALL
debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
              VkDebugUtilsMessageTypeFlagsEXT messageType,
              const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
              void* pUserData) {
    switch (messageSeverity) {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            FTRACE(pCallbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            FINFO(pCallbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            FWARN(pCallbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            FERROR(pCallbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT:
            break;
    }

    return VK_FALSE;
}

void populateDebugMessengerCreateInfo(
    VkDebugUtilsMessengerCreateInfoEXT* createInfo) {
    createInfo->sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo->messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo->messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                              VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                              VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo->pfnUserCallback = debugCallback;
}

b8 vulkanInit() {
    vulkanInfo.allocator = 0;
    createInstance();
    return true;
}

b8 vulkanDestroy() {
#ifdef _DEBUG
    DestroyDebugUtilsMessengerEXT(
        vulkanInfo.instance, vulkanInfo.debugMessenger, vulkanInfo.allocator);
#endif
    vkDestroyInstance(vulkanInfo.instance, vulkanInfo.allocator);
    return true;
}

b8 vulkanDraw() {
    return true;
}

void createInstance() {
    VkApplicationInfo va;
    va.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    va.pApplicationName = "Hello Triangle";
    va.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    va.pEngineName = "No Engine";
    va.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    va.apiVersion = VK_API_VERSION_1_0;
    va.pNext = 0;

    VkInstanceCreateInfo createInfo = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    createInfo.pApplicationInfo = &va;
    createInfo.enabledLayerCount = 0;
    createInfo.pNext = 0;

    const char** dinoExtStrings = dinoCreate(const char*);

#ifdef _DEBUG
    dinoPush(dinoExtStrings, &VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    const char** validationLayers = dinoCreate(const char*);
    dinoPush(validationLayers, &"VK_LAYER_KHRONOS_validation");

    u32 layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, 0);

    VkLayerProperties* availableLayers =
        dinoCreateReserveWithLengthSet(layerCount, VkLayerProperties);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers);

    for (u32 i = 0; i < dinoLength(validationLayers); i++) {
        b8 layerFound = false;
        FDEBUG("Validation Layer: %s", validationLayers[i]);

        for (u32 x = 0; x < dinoLength(availableLayers); x++) {
            // FDEBUG("Available Layer: %s", availableLayers[i]);
            if (strcmp(validationLayers[i], availableLayers[x].layerName) ==
                0) {
                FDEBUG("Found");
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            FERROR("Not all validationLayers found");
        }
    }

    createInfo.enabledLayerCount = dinoLength(validationLayers);
    createInfo.ppEnabledLayerNames = validationLayers;

    VkDebugUtilsMessengerCreateInfoEXT instanceDebugCI;
    populateDebugMessengerCreateInfo(&instanceDebugCI);
    createInfo.pNext = &instanceDebugCI;
#endif

    dinoPush(dinoExtStrings, &VK_KHR_SURFACE_EXTENSION_NAME);

    platformGetRequiredExtenstions(&dinoExtStrings);

    FINFO("Required Extensions:");

    for (i32 i = 0; i < dinoLength(dinoExtStrings); i++) {
        FINFO("\t %s", dinoExtStrings[i]);
    }

    uint32_t extensionCnt = 0;
    vkEnumerateInstanceExtensionProperties(0, &extensionCnt, 0);
    VkExtensionProperties* dinoExtDetails =
        dinoCreateReserve(extensionCnt, VkExtensionProperties);
    vkEnumerateInstanceExtensionProperties(0, &extensionCnt, dinoExtDetails);

    FINFO("Available Extensions:");

    for (i32 i = 0; i < dinoLength(dinoExtDetails); i++) {
        FINFO("\t %s", dinoExtDetails[i].extensionName);
    }

    createInfo.enabledExtensionCount = dinoLength(dinoExtStrings);
    createInfo.ppEnabledExtensionNames = dinoExtStrings;

    VkResult result = vkCreateInstance(&createInfo, vulkanInfo.allocator, &vulkanInfo.instance);
    if (result != VK_SUCCESS) {
        FERROR("Failed to create instance");
    }

#ifdef _DEBUG
    // Debug Messenger
    VkDebugUtilsMessengerCreateInfoEXT debugCI;
    populateDebugMessengerCreateInfo(&debugCI);

    VkResult res = CreateDebugUtilsMessengerEXT(vulkanInfo.instance, &debugCI,
                                                vulkanInfo.allocator,
                                                &vulkanInfo.debugMessenger);

    if (res != VK_SUCCESS) {
        FERROR("Could not create debug messenger");
    }
#endif

    dinoDestroy(dinoExtStrings);
    dinoDestroy(dinoExtDetails);
}
