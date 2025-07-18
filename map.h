#define MAP_WIDTH 10
#define MAP_HEIGHT 10
#include "player.h"
#include <stdio.h>
#include <stddef.h>
#include "enemy.h"

void drawMap(Player player, Enemy enemies[], int enemyCount );

void movePlayer(Player*p, char dir);