#include "player.h"
#include "enemy.h" // Ensure Enemy struct is defined
#include <stdlib.h> // for using rand()
#include <time.h>
#include "map.h"
#include "screen.h"


// Set starting values for player
void initEnemy(Enemy enemies[], int count)
{
    for (int i = 0; i < count; i++) {
        enemies[i].x = rand()% mapwidth;        // Starting x position
        enemies[i].y = rand()% mapHeight;        // Starting y position
        enemies[i].hp = 10;      // Health points
        enemies[i].alive = 1;    // enemy dead or alive (1 = alive, 0 = dead)
    }
}
