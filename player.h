#ifndef PLAYER_H
#define PLAYER_H

// Player info
typedef struct
{
    int x, y; // Position on map
    int hp;   // Health
    int xp;   // Experience
              // int level;     // Level
    char symble;
} Player;

// Set default values for player
void initPlayer(Player *player, int x, int y);
void movePlayer(Player *player, int dx, int dy, int mapWidth, int mapHeight); 

void drawPlayer(Player *player);
#endif // PLAYER_H
