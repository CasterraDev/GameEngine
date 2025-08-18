#include "core/systems/event.h"
#include "core/systems/logger.h"
#include "helpers/dinoarray.h"

typedef struct RegisteredEventPairing {
    void* listener;
    PF_OnEvent functionCallback;
} RegisteredEventPairing;

typedef struct EventCodeEntry {
    RegisteredEventPairing* events;
} EventCodeEntry;

// NOTE-BUG: May need to up this number
#define MAX_MESSAGE_CODES 10000

// Array of registered events
typedef struct EventSystemState {
    EventCodeEntry registered[MAX_MESSAGE_CODES];
} EventSystemState;

static EventSystemState* systemPtr;

b8 eventInit(u64* memoryRequirement, void* state) {
    *memoryRequirement = sizeof(EventSystemState);
    if (state == 0) {
        return true;
    }
    systemPtr = state;
    return true;
}

void eventShutdown() {
    if (!systemPtr) {
        FERROR("Event system was called before it was inited.")
    }
    // Free ALL THE DINOS.
    for (u16 i = 0; i < MAX_MESSAGE_CODES; ++i) {
        if (systemPtr->registered[i].events != 0) {
            dinoDestroy(systemPtr->registered[i].events);
            systemPtr->registered[i].events = 0;
        }
    }
    systemPtr = 0;
}

b8 eventRegister(u16 code, void* listener, PF_OnEvent onEvent) {
    if (!systemPtr) {
        FERROR("Event system was called before it was inited.")
        return false;
    }

    // Each event code has it's own dinoArray of listeners.
    // If it has zero listeners then create it
    if (systemPtr->registered[code].events == 0) {
        systemPtr->registered[code].events = dinoCreate(RegisteredEventPairing);
    }

    u64 registeredPairings = dinoLength(systemPtr->registered[code].events);
    for (u64 i = 0; i < registeredPairings; i++) {
        // If the pairing already exists don't add another. or the onEventFN
        // while get called twice confusing the user
        RegisteredEventPairing e = systemPtr->registered[code].events[i];
        if (e.listener == listener && e.functionCallback == onEvent) {
            FWARN("Event listener/onEvent pairing already added.");
            return false;
        }
    }

    RegisteredEventPairing event;
    event.listener = listener;
    event.functionCallback = onEvent;
    dinoPush(systemPtr->registered[code].events, event);

    return true;
}

b8 eventUnregister(u16 code, void* listener, PF_OnEvent onEvent) {
    if (!systemPtr) {
        FERROR("Event system was called before it was inited.")
        return false;
    }

    if (systemPtr->registered[code].events == 0) {
        FWARN("Event code has no listeners registered.");
        return false;
    }

    u64 registeredPairings = dinoLength(systemPtr->registered[code].events);
    for (u64 i = 0; i < registeredPairings; ++i) {
        RegisteredEventPairing e = systemPtr->registered[code].events[i];
        if (e.listener == listener && e.functionCallback == onEvent) {
            RegisteredEventPairing pairingToDel;
            dinoPopAt(systemPtr->registered[code].events, i, &pairingToDel);

            return true;
        }
    }

    return false;
}

b8 eventFire(u16 code, void* sender, EventContext context) {
    if (!systemPtr) {
        FERROR("Event system was called before it was inited.")
        return false;
    }

    // If their are no listeners. Return false so the gamedev can know
    if (systemPtr->registered[code].events == 0) {
        return false;
    }

    u64 registeredPairings = dinoLength(systemPtr->registered[code].events);
    for (u64 i = 0; i < registeredPairings; ++i) {
        RegisteredEventPairing e = systemPtr->registered[code].events[i];
        if (e.functionCallback(code, sender, e.listener, context)) {
            // Event has been handled, do not send to other listeners.
            return true;
        }
    }

    return false;
}
