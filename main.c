#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include "map.h"
#include "player.h"
#include "enemy.h"

int main(void)
{
    Player player;
    char input;
    initPlayer(&player);

    Enemy enemy;
    initEnemy(&enemy);

    while (1)
    {
        system("clear");
        drawMap(player, enemy);
        printf("\n use WASD to move, Qto quit:");
        scanf(" %c", &input);
        if (input=='q') break;
        movePlayer(&player, input);
        if (player.x == enemy.x && player.y == enemy.y && enemy.alive)
        {
            enemy.alive = 0;
            player.xp += 10;
        }
        
    }
    printf("thanks");
    return 0;
}