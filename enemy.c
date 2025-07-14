#include "player.h"
#include "enemy.h" // Ensure Enemy struct is defined

// Set starting values for player
void initEnemy(Enemy *e)
{
    e->x = 5;        // Starting x position
    e->y = 5;        // Starting y position
    e->hp = 10;      // Health points
    e->alive = 1;    // enemy dead or alive (1 = alive, 0 = dead)
}
