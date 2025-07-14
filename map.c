#include <stdio.h>
#include "map.h"
#include "player.h"
#include "enemy.h"

void drawMap(Player p, Enemy e)
{
    printf("HP: %d | XP: %d | Level: %d\n", p.hp, p.xp, p.level);

    for (size_t i = 0; i < MAP_HEIGHT; i++)
    {
        for (size_t j = 0; j < MAP_WIDTH; j++)
        {
            if (i == p.y && j == p.x)
            {
                printf("@ ");
            }
            if (i == e.y && j == e.x && e.alive == 1)
            {
                printf("E ");
            }

            else
            {
                printf(". ");
            }
        }
        printf("\n");
    }
}
void movePlayer(Player *p, char dir)
{
    switch (dir)
    {
    case 'w':
        if (p->y > 0)
        {
            p->y--;
        }

        break;
    case 's':
        if (p->y < MAP_HEIGHT - 1)
        {
            p->y++;
        }

        break;
    case 'a':
        if (p->x > 0)
        {
            p->x--;
        }

        break;
    case 'd':
        if (p->x < MAP_WIDTH - 1)
        {
            p->x++;
        }
        break;

    default:
        break;
    }
}