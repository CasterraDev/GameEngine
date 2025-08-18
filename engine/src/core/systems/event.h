#pragma once

#include "defines.h"

typedef struct EventContext {
    // For now we will give 128 bytes for events. If the user needs more they
    // can use pointers
    union {
        i64 i64[2];
        u64 u64[2];
        i32 i32[4];
        u32 u32[4];
        i16 i16[8];
        u16 u16[8];
        i8 i8[16];
        u8 u8[16];

        b32 b32[4];
        b8 b8[16];

        f64 f64[2];
        f32 f32[4];

        char c[16];
    } data;
} EventContext;

/**
 * @brief A Pointer Function (PF) for event functions to follow
 * @param eventCode The event code to be sent.
 * @param sender A pointer to the sender of the event. Can be 0/NULL.
 * @param listenerInstance A pointer to the listener of the event. Can be
 * 0/NULL.
 * @param data The event data to pass to the PF from the event when the event is
 * fired.
 * @returns Will return True if the message is considered handled (and will not
 * allow other event handler PFs to see it) otherwise false (and will allow
 * other PFs to see).
 */
typedef b8 (*PF_OnEvent)(u16 eventCode, void* sender, void* listenerInstance,
                         EventContext data);

/**
 * @brief Init the event system. Must be called twice once with no state (state
 * = 0) to get the memoryRequirement. Another after the state has been assigned
 * a memory block. (All systems work this way)
 * @param memoryRequirement out Variable that will tell you how much memory is
 * required for this system.
 * @param state Pointer to the block of memory that was allocated outside.
 * (Reason you need to call this FN twice)
 */
b8 eventInit(u64* memoryRequirement, void* state);

/**
 * @brief Shutsdown the event system.
 */
void eventShutdown();

/**
 * Register to listen for when events are sent with the provided code. Events
 * with duplicate `listener` & `onEvent` pairings will not be registered
 * and this FN will return false.
 * @param code The event code to listen for.
 * @param listener A pointer to a listener instance. Can be 0/NULL.
 * @param onEvent The callback function pointer to be invoked when the event
 * code is fired.
 * @returns true if the event is successfull
 */
CT_API b8 eventRegister(u16 code, void* listener, PF_OnEvent onEvent);

/**
 * Unregister from listening for when events are sent with the provided code. If
 * no `listener` & `onEvent` pairing is found, this FN returns false.
 * @param code The event code to stop listening for.
 * @param listener A pointer to a listener instance. Can be 0/NULL.
 * @param onEvent The callback function pointer to be unregistered.
 * @returns true if the event is successfull
 */
CT_API b8 eventUnregister(u16 code, void* listener, PF_OnEvent onEvent);

/**
 * Fires an event to listeners of the given code. If an event handler returns
 * true, the event is considered handled and is not passed on to any more
 * listeners.
 * @param code The event code to fire.
 * @param sender A pointer to the sender. Can be 0/NULL.
 * @param data The event data.
 * @returns Will return True if the message is considered handled (and will not
 * allow other event handler PFs to see it) otherwise false (and will allow
 * other PFs to see).
 */
CT_API b8 eventFire(u16 code, void* sender, EventContext context);

// TODO: Make the event system take strings instead of enums. So users can make
// custom user-defined events. Will need to use a hashmap tho. For these codes
// use that preprocessor trick to make an enum & string array at the same time.
typedef enum systemEventCode {
    // ENGINE LEVEL EVENTS

    /** @brief Tells the application to shutdown */
    EVENT_CODE_APPLICATION_QUIT,

    /** @brief Keyboard key pressed.
     * u16 keyCode = data.u16[0];
     */
    EVENT_CODE_KEY_PRESSED,

    /** @brief Keyboard key released.
     * u16 keyCode = data.u16[0];
     */
    EVENT_CODE_KEY_RELEASED,

    /** @brief Keyboard key held down.
     * u16 keyCode = data.u16[0];
     */
    EVENT_CODE_KEY_DOWN,

    /** @brief Mouse button pressed.
     * u16 button = data.u16[0];
     */
    EVENT_CODE_BUTTON_PRESSED,

    /** @brief Mouse button released.
     * u16 button = data.u16[0];
     */
    EVENT_CODE_BUTTON_RELEASED,

    /** @brief Mouse button held down.
     * u16 button = data.u16[0];
     */
    EVENT_CODE_BUTTON_DOWN,

    /** @brief Mouse moved.
     * u16 x = data.i16[0];
     * u16 y = data.i16[1];
     */
    EVENT_CODE_MOUSE_MOVED,

    /** @brief Mouse moved.
     * ui zDelta = data.i8[0];
     */
    EVENT_CODE_MOUSE_WHEEL,

    /** @brief Resized/resolution changed from the OS.
     * u16 width = data.u16[0];
     * u16 height = data.u16[1];
     */
    EVENT_CODE_RESIZED,

    /** @brief Keyboard key up/not held down.
     * u16 keyCode = data.u16[0];
     */
    EVENT_CODE_KEY_UP,

    /** @brief Debugging Event. Shouldn't be used in production/release */
    EVENT_CODE_DEBUG0,
    /** @brief Debugging Event. Shouldn't be used in production/release */
    EVENT_CODE_DEBUG1,
    /** @brief Debugging Event. Shouldn't be used in production/release */
    EVENT_CODE_DEBUG2,

    /** Used as a hack to for-loop enums. Users shouldn't need this */
    EVENT_CODE_MAX_TAGS
} systemEventCode;
