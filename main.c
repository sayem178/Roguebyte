#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <time.h>
#include "map.h"
#include "player.h"
#include "enemy.h"
#include "screen.h"

int main(void)
{

    srand(time(NULL));
    Player player;
    char input;
    initPlayer(&player);

    Enemy enemies[MAX_ENEMIES];
    initEnemy(enemies, MAX_ENEMIES);

    while (1)
    {
        system("clear");
        drawMap(player, enemies, MAX_ENEMIES);
        printf("\n use WASD to move, Qto quit:");
        scanf(" %c", &input);
        if (input == 'q')
            break;
        movePlayer(&player, input);
        for (int i = 0; i < MAX_ENEMIES; i++) // to check if the player has bumped into any of the enemies
        {
            if (player.x == enemies[i].x && player.y == enemies[i].y && enemies[i].alive)
            {
                enemies[i].alive = 0;
                player.xp += 10;
            }
        }
        
    }
    printf("thanks");
    return 0;
}