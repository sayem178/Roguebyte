#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include "map.h"
#include "player.h"
int main(void)
{
    Player player;
    char input;
    initPlayer(&player);

    while (1)
    {
        system("clear");
        drawMap(player);
        printf("\n use WASD to move, Qto quit:");
        scanf(" %c", &input);
        if (input=='q') break;
        movePlayer(&player, input);
    }
    printf("thanks");
    return 0;
}