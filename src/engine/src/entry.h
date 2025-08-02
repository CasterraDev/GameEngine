#include "core/engine.h"
#include "core/logger.h"
#include "defines.h"

extern b8 createGame();

int main (int argc, char *argv[]) {
    if (!createGame()){
        FFATAL("Failed to create game.");
        return -1;
    }

    if (!startEngine()){
        FFATAL("Failed to start engine.");
        return -1;
    }

    return 0;
}
