#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "map.h"

void generateRow(char *row, int width)
{
    for (int i = 0; i < width; i++)
    {
        row[i] = (rand() % 10 == 0) ? '#' : ' '; // 10% chance
    }
    row[width] = '\0';
}

void initMap(Map *map, int width, int height)
{

    map->width = width;
    map->height = height;
    map->scroll_offset = 0;
    map->rows = (char **)malloc(sizeof(char *) * MAX_ROWS);

    for (int i = 0; i < MAX_ROWS; i++)
    {
        map->rows[i] = (char *)malloc(sizeof(char) * (width + 1));
        generateRow(map->rows[i], width);
    }
}

void scrollMap(Map *map)
{
    map->scroll_offset++;
    if (map->scroll_offset + map->height >= MAX_ROWS)
    {
        map->scroll_offset = 0; // loop back
    }
}
void drawMap(Map *map)
{

    for (int i = 0; i < map->height; i++)
    {
        printf("%s\n", map->rows[map->scroll_offset + i]);
    }
}
void freeMap(Map *map)
{
    for (int i = 0; i < MAX_ROWS; i++)
    {
        free(map->rows[i]);
    }
    free(map->rows);
}
