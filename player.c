#include "player.h"
#include "map.h"
#include "screen.h"
// Set starting values for player
void initPlayer(Player *player, int x, int y)
{
    player->x = x;    // Starting x position
    player->y = y;    // Starting y position
    player->hp = 100; // Health points
    player->xp = 0;   // Experience points
    // player->level = 1;    // Player level
    player->symble = '@';
}
void movePlayer(Player *player, int dx, int dy, int mapWidth, int mapHeight)
{
    int newX = player->x + dx;
    int newY = player->y + dy;
    if (newX >= 0 && newX < mapWidth && newY >= 0 && newY < mapHeight)
    {
        player->x = newX;
        player->y = newY;
    }
}
void drawPlayer(Player *player)
{

    moveCursor(player->x, player->y);
    printf("%c", player->symble);
}
void clearPlayer(Player *player)
{
    moveCursor(player->x, player->y);
    printf(" ");
}
