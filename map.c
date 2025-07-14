#include <stdio.h>
#include "map.h"
#include "player.h"
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