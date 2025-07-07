#ifndef PLAYER_H
#define PLAYER_H

// Player info
typedef struct {
    int x, y;      // Position on map
    int hp;        // Health
    int xp;        // Experience
    int level;     // Level
} Player;

// Set default values for player
void initPlayer(Player *p);

#endif // PLAYER_H
