#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> //to get access to system-level functions or get access to low level od powers on linux
#include <time.h>
#include "map.h"
#include "screen.h"
#include "player.h"
int main(void)
{

    srand(time(NULL)); // for random seeds so the obstacles aren't same

    int rows, cols;
    if (getTerminalSize(&rows, &cols) == -1) // get terminal size
    {
        printf("Error getting terminal size.\n");
        return 1;
    }

    int mapHeight = rows; // 2 rows for UI maybe?
    int mapwidth = cols;

    Map gameMap;
    initMap(&gameMap, mapwidth, mapHeight); // initialize map
    Player player;
    initPlayer(&player, mapwidth, mapHeight);

    while (1)
    {
        system("clear");
        drawMap(&gameMap);   // makes the map visiable
        scrollMap(&gameMap); // moves the map vertical scrolling
        usleep(100000);      // slows the loop to make it visible
    }

    freeMap(&gameMap); // frees allocated memory from initmap
    return 0;
}