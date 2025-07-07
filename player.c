#include "player.h"

// Set starting values for player
void initPlayer(Player *p)
{
    p->x = 1;        // Starting x position
    p->y = 1;        // Starting y position
    p->hp = 100;     // Health points
    p->xp = 0;       // Experience points
    p->level = 1;    // Player level
}
