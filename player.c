#include "player.h"

// Set starting values for player
void initPlayer(Player *player)
{
    player->x = 1;        // Starting x position
    player->y = 1;        // Starting y position
    player->hp = 100;     // Health points
    player->xp = 0;       // Experience points
    player->level = 1;    // Player level
}
