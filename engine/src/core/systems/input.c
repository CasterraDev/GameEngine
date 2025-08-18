#include "core/systems/input.h"
#include "core/systems/event.h"
#include "core/systems/fmemory.h"
#include "core/systems/logger.h"

typedef struct keyState {
    b8 isDown;
} keyState;

typedef struct keyboardState {
    keyState keys[256];
} keyboardState;

typedef struct buttonState {
    b8 isDown;
} buttonState;

typedef struct mouseState {
    i16 x;
    i16 y;
    buttonState buttons[BUTTON_MAX_BUTTONS];
} mouseState;

typedef struct inputState {
    keyboardState keyboardCur;
    keyboardState keyboardPrev;
    mouseState mouseCur;
    mouseState mousePrev;
} inputState;

static inputState* systemPtr;

void inputInit(u64* memoryRequirement, void* state) {
    *memoryRequirement = sizeof(inputState);
    if (state == 0) {
        return;
    }
    systemPtr = state;

    FINFO("Input system inited");
}

void inputUpdate(f64 deltaTime) {
    if (!systemPtr) {
        FERROR("Input system is not inited");
        return;
    }

    fcpyMem(&systemPtr->keyboardPrev, &systemPtr->keyboardCur,
            sizeof(keyboardState));
    fcpyMem(&systemPtr->mousePrev, &systemPtr->mouseCur, sizeof(mouseState));
}

void inputShutdown(void* state) {
    systemPtr = 0;
}

void inputProcessKey(GE_Keys key, b8 pressed) {
    EventContext context;
    context.data.u16[0] = key;
    systemPtr->keyboardCur.keys[key].isDown = pressed;

    if (systemPtr->keyboardCur.keys[key].isDown !=
        systemPtr->keyboardPrev.keys[key].isDown) {
        eventFire(pressed ? EVENT_CODE_KEY_PRESSED : EVENT_CODE_KEY_RELEASED, 0,
                  context);
    } else {
        eventFire(EVENT_CODE_KEY_DOWN, 0, context);
    }
}

void inputProcessButton(GE_Buttons button, b8 pressed) {
    EventContext context;
    context.data.u16[0] = button;
    systemPtr->mouseCur.buttons[button].isDown = pressed;
    if (systemPtr->mouseCur.buttons[button].isDown !=
        systemPtr->mousePrev.buttons[button].isDown) {
        eventFire(pressed ? EVENT_CODE_BUTTON_PRESSED
                          : EVENT_CODE_BUTTON_RELEASED,
                  0, context);
    } else {
        eventFire(EVENT_CODE_BUTTON_DOWN, 0, context);
    }
}

void inputProcessMouseMove(i16 x, i16 y) {
    // Only process if actually different
    if (systemPtr->mouseCur.x != x || systemPtr->mouseCur.y != y) {
        systemPtr->mouseCur.x = x;
        systemPtr->mouseCur.y = y;

        EventContext context;
        context.data.u16[0] = x;
        context.data.u16[1] = y;
        eventFire(EVENT_CODE_MOUSE_MOVED, 0, context);
    }
}

void inputProcessMouseWheel(i8 zDelta) {
    EventContext context;
    context.data.u8[0] = zDelta;
    eventFire(EVENT_CODE_MOUSE_WHEEL, 0, context);
}

b8 inputIsKeyDown(GE_Keys key) {
    if (!systemPtr) {
        FERROR("Input system is not inited");
        return false;
    }
    return systemPtr->keyboardCur.keys[key].isDown;
}

b8 inputIsKeyUp(GE_Keys key) {
    if (!systemPtr) {
        FERROR("Input system is not inited");
        return true;
    }
    return !systemPtr->keyboardCur.keys[key].isDown;
}

b8 inputIsKeyReleased(GE_Keys key) {
    if (!systemPtr) {
        FERROR("Input system is not inited");
        return false;
    }
    return ((systemPtr->keyboardPrev.keys[key].isDown !=
             systemPtr->keyboardCur.keys[key].isDown) &&
            !systemPtr->keyboardCur.keys[key].isDown);
}

b8 inputIsKeyPressed(GE_Keys key) {
    if (!systemPtr) {
        FERROR("Input system is not inited");
        return false;
    }
    return ((systemPtr->keyboardPrev.keys[key].isDown !=
             systemPtr->keyboardCur.keys[key].isDown) &&
            systemPtr->keyboardCur.keys[key].isDown);
}

b8 inputWasKeyPressed(GE_Keys key) {
    if (!systemPtr) {
        FERROR("Input system is not inited");
        return false;
    }
    return systemPtr->keyboardPrev.keys[key].isDown;
}

b8 inputWasKeyUp(GE_Keys key) {
    if (!systemPtr) {
        FERROR("Input system is not inited");
        return true;
    }
    return !systemPtr->keyboardPrev.keys[key].isDown;
}

// mouse input
b8 inputIsButtonPressed(GE_Buttons button) {
    if (!systemPtr) {
        FERROR("Input system is not inited");
        return false;
    }
    return ((systemPtr->mouseCur.buttons[button].isDown !=
             systemPtr->mousePrev.buttons[button].isDown) &&
            systemPtr->mouseCur.buttons[button].isDown);
}

b8 inputIsButtonReleased(GE_Buttons button) {
    if (!systemPtr) {
        FERROR("Input system is not inited");
        return false;
    }
    return ((systemPtr->mouseCur.buttons[button].isDown !=
             systemPtr->mousePrev.buttons[button].isDown) &&
            !systemPtr->mouseCur.buttons[button].isDown);
}

b8 inputIsButtonUp(GE_Buttons button) {
    if (!systemPtr) {
        FERROR("Input system is not inited");
        return true;
    }
    return !systemPtr->mouseCur.buttons[button].isDown;
}

b8 inputIsButtonDown(GE_Buttons button) {
    if (!systemPtr) {
        FERROR("Input system is not inited");
        return true;
    }
    return systemPtr->mouseCur.buttons[button].isDown;
}

b8 inputWasButtonPressed(GE_Buttons button) {
    if (!systemPtr) {
        FERROR("Input system is not inited");
        return false;
    }
    return systemPtr->mousePrev.buttons[button].isDown;
}

b8 inputWasButtonUp(GE_Buttons button) {
    if (!systemPtr) {
        FERROR("Input system is not inited");
        return true;
    }
    return !systemPtr->mousePrev.buttons[button].isDown;
}

void inputGetMousePosition(i32* x, i32* y) {
    if (!systemPtr) {
        FERROR("Input system is not inited");
        *x = 0;
        *y = 0;
        return;
    }
    *x = systemPtr->mouseCur.x;
    *y = systemPtr->mouseCur.y;
}

void inputGetPreviousMousePosition(i32* x, i32* y) {
    if (!systemPtr) {
        FERROR("Input system is not inited");
        *x = 0;
        *y = 0;
        return;
    }
    *x = systemPtr->mousePrev.x;
    *y = systemPtr->mousePrev.y;
}
