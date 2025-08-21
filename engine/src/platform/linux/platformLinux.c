#include "helpers/dinoarray.h"
#include "platform/platform.h"

// Linux platform.
#if GE_PLATFORM_LINUX

#include "core/systems/event.h"
#include "core/systems/input.h"
#include "core/systems/logger.h"

#include <X11/XKBlib.h>   // system install libx11-dev
#include <X11/Xlib-xcb.h> // system install libxkbcommon-x11-dev
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <sys/time.h>
#include <xcb/xcb.h>

#if _POSIX_C_SOURCE >= 199309L
#include <time.h> // nanosleep
#else
#include <unistd.h> // usleep
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * XLib - https://www.x.org/releases/current/doc/libX11/libX11/libX11.html
 * XLib & XCB - https://www.x.org/wiki/guide/xlib-and-xcb/#index2h2
 * XCB - https://xcb.freedesktop.org/
 * XCB Tutorials - https://xcb.freedesktop.org/tutorial/
 */

typedef struct platformState {
    Display* display;
    xcb_connection_t* connection;
    xcb_window_t window;
    xcb_screen_t* screen;
    xcb_atom_t wm_protocols;
    xcb_atom_t wm_delete_win;
} platformState;

static platformState* systemPtr;

GE_Keys translateXKeysToMyKeys(u32 x_keycode);

static void testCookie(xcb_void_cookie_t cookie, xcb_connection_t* connection,
                       char* errMessage) {
    xcb_generic_error_t* error = xcb_request_check(connection, cookie);
    if (error) {
        FERROR("%s : %d", errMessage, error->error_code);
        xcb_disconnect(connection);
        exit(-1);
    }
}

b8 platformInit(u64* memoryRequirement, void* state) {
    *memoryRequirement = sizeof(platformState);
    if (state == 0) {
        return true;
    }
    systemPtr = state;
    return true;
}

b8 platformStartup(const char* appName, i32 x, i32 y, i32 width, i32 height) {
    if (!systemPtr) {
        FERROR("platformStartup called before platform system was inited")
    }
    // Open a connection to the X server
    // passing NULL will make it grab the default screen using the DISPLAY env
    // variable
    systemPtr->display = XOpenDisplay(NULL);

    // TODO: Maybe turn this on.
    // NOTE: if the app crashes key repeats stay off
    // XAutoRepeatOff(systemPtr->display);

    // Get a connection that xcb can use from xlib - need libx11-xcb
    systemPtr->connection = XGetXCBConnection(systemPtr->display);

    if (xcb_connection_has_error(systemPtr->connection)) {
        FFATAL("Failed to get xcb connection from XLib");
        return false;
    }

    const struct xcb_setup_t* setup = xcb_get_setup(systemPtr->connection);

    // Loop through screens using iterator
    xcb_screen_iterator_t it = xcb_setup_roots_iterator(setup);
    int screenPtr = 0;
    for (i32 s = screenPtr; s > 0; s--) {
        xcb_screen_next(&it);
    }

    systemPtr->screen = it.data;

    // Give the window an ID
    systemPtr->window = xcb_generate_id(systemPtr->connection);

    // Register event types.
    // XCB_CW_BACK_PIXEL = BG color for the window
    // XCB_CW_EVENT_MASK is required unless you have no event masks.
    u32 eventMask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;

    // Listen for keyboard and mouse buttons
    u32 eventVals = XCB_EVENT_MASK_BUTTON_PRESS |
                    XCB_EVENT_MASK_BUTTON_RELEASE | XCB_EVENT_MASK_KEY_PRESS |
                    XCB_EVENT_MASK_KEY_RELEASE | XCB_EVENT_MASK_EXPOSURE |
                    XCB_EVENT_MASK_POINTER_MOTION |
                    XCB_EVENT_MASK_STRUCTURE_NOTIFY;

    u32 valList[] = {systemPtr->screen->black_pixel, eventVals};

    // Create the window
    xcb_void_cookie_t cookie =
        xcb_create_window(systemPtr->connection,
                          XCB_COPY_FROM_PARENT, // depth
                          systemPtr->window,
                          systemPtr->screen->root, // parent
                          x, y, width, height,
                          0,                             // No border
                          XCB_WINDOW_CLASS_INPUT_OUTPUT, // class
                          systemPtr->screen->root_visual, eventMask, valList);

    testCookie(cookie, systemPtr->connection, "Could not create window");

    // Change the title
    xcb_change_property(systemPtr->connection, XCB_PROP_MODE_REPLACE,
                        systemPtr->window, XCB_ATOM_WM_NAME, XCB_ATOM_STRING,
                        8, // data should be viewed 8 bits at a time
                        strlen(appName), appName);

    // Tell the server to notify when the window manager
    // attempts to destroy the window.
    // https://www.systutorials.com/docs/linux/man/3-xcb_intern_atom/
    // https://lists.freedesktop.org/archives/xcb/2010-December/006714.html
    xcb_intern_atom_cookie_t wm_delete_cookie =
        xcb_intern_atom(systemPtr->connection, false,
                        strlen("WM_DELETE_WINDOW"), "WM_DELETE_WINDOW");

    xcb_intern_atom_cookie_t wm_protocols_cookie = xcb_intern_atom(
        systemPtr->connection, false, strlen("WM_PROTOCOLS"), "WM_PROTOCOLS");

    xcb_intern_atom_reply_t* wm_delete_reply =
        xcb_intern_atom_reply(systemPtr->connection, wm_delete_cookie, NULL);
    xcb_intern_atom_reply_t* wm_protocols_reply =
        xcb_intern_atom_reply(systemPtr->connection, wm_protocols_cookie, NULL);

    // Save these cuz pump messages will most likely need them
    systemPtr->wm_delete_win = wm_delete_reply->atom;
    systemPtr->wm_protocols = wm_protocols_reply->atom;

    xcb_change_property(systemPtr->connection, XCB_PROP_MODE_REPLACE,
                        systemPtr->window, wm_protocols_reply->atom, 4, 32, 1,
                        &wm_delete_reply->atom);

    // Map the window to the screen
    xcb_map_window(systemPtr->connection, systemPtr->window);

    // Flush the stream
    i32 flushRes = xcb_flush(systemPtr->connection);
    if (flushRes <= 0) {
        FFATAL("Error flushing xcb: %d", flushRes);
        return false;
    }
    FINFO("Platform inited");

    return true;
}

void platformShutdown() {
    if (systemPtr) {
        // If connection is there display & window should also be made
        if (systemPtr->connection) {
            // Turn key repeats back on
            // This is global for linux. Which is ... weird
            // XAutoRepeatOn(systemPtr->display);

            xcb_destroy_window(systemPtr->connection, systemPtr->window);
        }
        systemPtr = 0;
    }
}

void platformGetRequiredExtenstions(const char*** dinoStrings) {
    dinoPush(*dinoStrings, &"VK_KHR_xcb_surface");
}

b8 platformPumpMessages() {
    xcb_generic_event_t* event;
    xcb_client_message_event_t* cm;

    b8 quitFlagged = false;

    while ((event = xcb_poll_for_event(systemPtr->connection))) {
        // FDEBUG("XCB Message %d",
        //        ((xcb_client_message_event_t*)event)->data.data32[0]);

        // Input events
        switch (event->response_type & ~0x80) {
            case XCB_KEY_PRESS:
            case XCB_KEY_RELEASE: {
                xcb_key_press_event_t* keyPressEvent =
                    (xcb_key_press_event_t*)event;
                b8 pressed = event->response_type == XCB_KEY_PRESS;
                xcb_keycode_t code = keyPressEvent->detail;
                KeySym keySym =
                    XkbKeycodeToKeysym(systemPtr->display, (KeyCode)code, 0,
                                       code & ShiftMask ? 1 : 0);

                GE_Keys key = translateXKeysToMyKeys(keySym);

                inputProcessKey(key, pressed);
                break;
            }
            case XCB_BUTTON_PRESS:
            case XCB_BUTTON_RELEASE: {
                xcb_button_press_event_t* mouseButtonEvent =
                    (xcb_button_press_event_t*)event;
                b8 pressed = event->response_type == XCB_BUTTON_PRESS;
                GE_Buttons curButton = BUTTON_MAX_BUTTONS;
                switch (mouseButtonEvent->detail) {
                    case XCB_BUTTON_INDEX_1:
                        curButton = BUTTON_LEFT;
                        break;
                    case XCB_BUTTON_INDEX_2:
                        curButton = BUTTON_MIDDLE;
                        break;
                    case XCB_BUTTON_INDEX_3:
                        curButton = BUTTON_RIGHT;
                        break;
                }

                if (curButton != BUTTON_MAX_BUTTONS) {
                    inputProcessButton(curButton, pressed);
                }
                break;
            }
            case XCB_MOTION_NOTIFY: {
                // Mouse move
                xcb_motion_notify_event_t* mouseMotionEvent =
                    (xcb_motion_notify_event_t*)event;

                // Pass over to the input subsystem.
                inputProcessMouseMove(mouseMotionEvent->event_x,
                                      mouseMotionEvent->event_y);
                break;
            }

            case XCB_CONFIGURE_NOTIFY: {
                // NOTE: This gets called when a window-move happens aswell
                xcb_configure_notify_event_t* cfgNotifyEvent =
                    (xcb_configure_notify_event_t*)event;

                EventContext ec;
                ec.data.u16[0] = cfgNotifyEvent->width;
                ec.data.u16[1] = cfgNotifyEvent->height;
                eventFire(EVENT_CODE_RESIZED, 0, ec);
                break;
            }

            case XCB_CLIENT_MESSAGE: {
                cm = (xcb_client_message_event_t*)event;

                // Window close
                if (cm->data.data32[0] == systemPtr->wm_delete_win) {
                    FDEBUG("Window Closed");
                    quitFlagged = true;
                    EventContext ec;
                    ec.data.u8[0] = true;
                    eventFire(EVENT_CODE_APPLICATION_QUIT, 0, ec);
                }
                break;
            }
            default:
                break;
        }

        free(event);
    }
    return !quitFlagged;
}

void* platformAllocate(u64 size, b8 aligned) {
    return malloc(size);
}
void platformFree(void* block, b8 aligned) {
    free(block);
}
void* platformZeroMemory(void* block, u64 size) {
    return memset(block, 0, size);
}
void* platformCopyMemory(void* dest, const void* source, u64 size) {
    return memcpy(dest, source, size);
}
void* platformSetMemory(void* dest, i32 value, u64 size) {
    return memset(dest, value, size);
}

void platformConsoleWrite(const char* message, u8 color) {
    // FATAL,ERROR,WARN,INFO,DEBUG,TRACE
    const char* colorStrs[] = {"0;41", "1;31", "1;33", "1;32", "1;34", "1;30"};
    printf("\033[%sm%s\033[0m", colorStrs[color], message);
}

void platformConsoleWriteError(const char* message, u8 color) {
    // Linux has the same logging for errors & non-errors
    platformConsoleWrite(message, color);
}

f64 platformGetAbsoluteTime() {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return now.tv_sec + now.tv_nsec * 0.000000001;
}

void platformSleep(u64 ms) {
#if _POSIX_C_SOURCE >= 199309L
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000 * 1000;
    nanosleep(&ts, 0);
#else
    if (ms >= 1000) {
        sleep(ms / 1000);
    }
    usleep((ms % 1000) * 1000);
#endif
}

// Key translation
GE_Keys translateXKeysToMyKeys(u32 x_keycode) {
    switch (x_keycode) {
        case XK_BackSpace:
            return KEY_BACKSPACE;
        case XK_Return:
            return KEY_ENTER;
        case XK_Tab:
            return KEY_TAB;
            // case XK_Shift: return KEY_SHIFT;
            // case XK_Control: return KEY_CONTROL;

        case XK_Pause:
            return KEY_PAUSE;
        case XK_Caps_Lock:
            return KEY_CAPITAL;

        case XK_Escape:
            return KEY_ESCAPE;

            // Not supported
            // case : return KEY_CONVERT;
            // case : return KEY_NONCONVERT;
            // case : return KEY_ACCEPT;

        case XK_Mode_switch:
            return KEY_MODECHANGE;

        case XK_space:
            return KEY_SPACE;
        case XK_Prior:
            return KEY_PRIOR;
        case XK_Next:
            return KEY_NEXT;
        case XK_End:
            return KEY_END;
        case XK_Home:
            return KEY_HOME;
        case XK_Left:
            return KEY_LEFT;
        case XK_Up:
            return KEY_UP;
        case XK_Right:
            return KEY_RIGHT;
        case XK_Down:
            return KEY_DOWN;
        case XK_Select:
            return KEY_SELECT;
        case XK_Print:
            return KEY_PRINT;
        case XK_Execute:
            return KEY_EXECUTE;
        // case XK_snapshot: return KEY_SNAPSHOT; // not supported
        case XK_Insert:
            return KEY_INSERT;
        case XK_Delete:
            return KEY_DELETE;
        case XK_Help:
            return KEY_HELP;

        case XK_Meta_L:
            return KEY_LWIN; // TODO: not sure this is right
        case XK_Meta_R:
            return KEY_RWIN;
            // case XK_apps: return KEY_APPS; // not supported

            // case XK_sleep: return KEY_SLEEP; //not supported

        case XK_KP_0:
            return KEY_NUMPAD0;
        case XK_KP_1:
            return KEY_NUMPAD1;
        case XK_KP_2:
            return KEY_NUMPAD2;
        case XK_KP_3:
            return KEY_NUMPAD3;
        case XK_KP_4:
            return KEY_NUMPAD4;
        case XK_KP_5:
            return KEY_NUMPAD5;
        case XK_KP_6:
            return KEY_NUMPAD6;
        case XK_KP_7:
            return KEY_NUMPAD7;
        case XK_KP_8:
            return KEY_NUMPAD8;
        case XK_KP_9:
            return KEY_NUMPAD9;
        case XK_multiply:
            return KEY_MULTIPLY;
        case XK_KP_Add:
            return KEY_ADD;
        case XK_KP_Separator:
            return KEY_SEPARATOR;
        case XK_KP_Subtract:
            return KEY_SUBTRACT;
        case XK_KP_Decimal:
            return KEY_DECIMAL;
        case XK_KP_Divide:
            return KEY_DIVIDE;
        case XK_F1:
            return KEY_F1;
        case XK_F2:
            return KEY_F2;
        case XK_F3:
            return KEY_F3;
        case XK_F4:
            return KEY_F4;
        case XK_F5:
            return KEY_F5;
        case XK_F6:
            return KEY_F6;
        case XK_F7:
            return KEY_F7;
        case XK_F8:
            return KEY_F8;
        case XK_F9:
            return KEY_F9;
        case XK_F10:
            return KEY_F10;
        case XK_F11:
            return KEY_F11;
        case XK_F12:
            return KEY_F12;
        case XK_F13:
            return KEY_F13;
        case XK_F14:
            return KEY_F14;
        case XK_F15:
            return KEY_F15;
        case XK_F16:
            return KEY_F16;
        case XK_F17:
            return KEY_F17;
        case XK_F18:
            return KEY_F18;
        case XK_F19:
            return KEY_F19;
        case XK_F20:
            return KEY_F20;
        case XK_F21:
            return KEY_F21;
        case XK_F22:
            return KEY_F22;
        case XK_F23:
            return KEY_F23;
        case XK_F24:
            return KEY_F24;

        case XK_Num_Lock:
            return KEY_NUMLOCK;
        case XK_Scroll_Lock:
            return KEY_SCROLL;

        case XK_KP_Equal:
            return KEY_NUMPAD_EQUAL;

        case XK_Shift_L:
            return KEY_LSHIFT;
        case XK_Shift_R:
            return KEY_RSHIFT;
        case XK_Control_L:
            return KEY_LCONTROL;
        case XK_Control_R:
            return KEY_RCONTROL;
        case XK_Alt_L:
            return KEY_LALT;
        case XK_Alt_R:
            return KEY_RALT;

        case XK_semicolon:
            return KEY_SEMICOLON;
        case XK_plus:
            return KEY_PLUS;
        case XK_comma:
            return KEY_COMMA;
        case XK_minus:
            return KEY_MINUS;
        case XK_period:
            return KEY_PERIOD;
        case XK_slash:
            return KEY_SLASH;
        case XK_grave:
            return KEY_GRAVE;

        case XK_a:
        case XK_A:
            return KEY_A;
        case XK_b:
        case XK_B:
            return KEY_B;
        case XK_c:
        case XK_C:
            return KEY_C;
        case XK_d:
        case XK_D:
            return KEY_D;
        case XK_e:
        case XK_E:
            return KEY_E;
        case XK_f:
        case XK_F:
            return KEY_F;
        case XK_g:
        case XK_G:
            return KEY_G;
        case XK_h:
        case XK_H:
            return KEY_H;
        case XK_i:
        case XK_I:
            return KEY_I;
        case XK_j:
        case XK_J:
            return KEY_J;
        case XK_k:
        case XK_K:
            return KEY_K;
        case XK_l:
        case XK_L:
            return KEY_L;
        case XK_m:
        case XK_M:
            return KEY_M;
        case XK_n:
        case XK_N:
            return KEY_N;
        case XK_o:
        case XK_O:
            return KEY_O;
        case XK_p:
        case XK_P:
            return KEY_P;
        case XK_q:
        case XK_Q:
            return KEY_Q;
        case XK_r:
        case XK_R:
            return KEY_R;
        case XK_s:
        case XK_S:
            return KEY_S;
        case XK_t:
        case XK_T:
            return KEY_T;
        case XK_u:
        case XK_U:
            return KEY_U;
        case XK_v:
        case XK_V:
            return KEY_V;
        case XK_w:
        case XK_W:
            return KEY_W;
        case XK_x:
        case XK_X:
            return KEY_X;
        case XK_y:
        case XK_Y:
            return KEY_Y;
        case XK_z:
        case XK_Z:
            return KEY_Z;

        default:
            return 0;
    }
}

#endif
