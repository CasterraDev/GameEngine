#include "core/engine.h"
#include "core/systems/logger.h"
#include "defines.h"
#include "gameInfo.h"

extern b8 createGame(GameInfo* gameInfo);

int main(int argc, char* argv[]) {
    GameInfo gameInfo;
    if (!createGame(&gameInfo)) {
        FFATAL("Failed to create game.");
        return -1;
    }

    if (!engineStart(&gameInfo)) {
        FFATAL("Failed to start engine.");
        return -1;
    }

    if (!engineRun(&gameInfo)) {
        FFATAL("Failed to run engine.");
        return -1;
    }

    if (!engineDestroy(&gameInfo)) {
        FFATAL("Failed to destroy engine.");
        return -1;
    }

    return 0;
}
