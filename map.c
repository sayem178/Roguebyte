#include <stdio.h>
#include "map.h"
#include "player.h"
#include "enemy.h"

void drawMap(Player player, Enemy enemies[], int enemyCount)
{
    printf("HP: %d | XP: %d | Level: %d\n", player.hp, player.xp, player.level);

    for (size_t i = 0; i < MAP_HEIGHT; i++)
    {
        for (size_t j = 0; j < MAP_WIDTH; j++)
        {
            if (i == player.y && j == player.x)
            {
                printf("@ ");
            }
            else
            {
                int enemyFound = 0;
                for (int k = 0; k < enemyCount; k++)
                {
                    if (i == enemies[k].y && j == enemies[k].x && enemies[k].alive == 1)
                    {
                        printf("E ");
                        enemyFound = 1;
                        break;
                    }
                }
                if (enemyFound)
                {
                    // Enemy was printed, skip printing '.'
                }
                else
                {
                    printf(". ");
                }
            }
        }
        printf("\n");
    }
}
void movePlayer(Player *player, char dir)
{
    switch (dir)
    {
    case 'w':
        if (player->y > 0)
        {
            player->y--;
        }

        break;
    case 's':
        if (player->y < MAP_HEIGHT - 1)
        {
            player->y++;
        }

        break;
    case 'a':
        if (player->x > 0)
        {
            player->x--;
        }

        break;
    case 'd':
        if (player->x < MAP_WIDTH - 1)
        {
            player->x++;
        }
        break;

    default:
        break;
    }
}