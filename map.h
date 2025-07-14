#define MAP_WIDTH 10
#define MAP_HEIGHT 10
#include "player.h"
#include <stdio.h>
#include <stddef.h>

void drawMap(Player p)
{
    for (size_t i = 0; i < MAP_HEIGHT; i++)
    {
        for (size_t j = 0; j < MAP_WIDTH; j++)
        {
            if (i == p.y && j == p.x)
            {
                printf("@");
            }
            else
            {
                printf(".");
            }
        }
        printf("\n");
    }
}
void movePlayer(Player*p, char dir);