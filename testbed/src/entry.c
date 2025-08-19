#include "core/systems/logger.h"
#include "gameInfo.h"
#include <entry.h>

b8 createGame(GameInfo* gameInfo) {
    // TODO: Application configuration.
    FINFO("Game Created.");
    gameInfo->appName = "Triangle";
    gameInfo->x = 0;
    gameInfo->y = 0;
    gameInfo->width = 50 * 16;
    gameInfo->height = 50 * 9;

    return true;
}
