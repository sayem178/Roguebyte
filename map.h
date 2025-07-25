#ifndef MAP_H
#define MAP_H
#define MAX_ROWS 1000

// to hold map layout
typedef struct
{
    char **rows; // pointer to pointer to store rows dynamically
    int width;
    int height;
    int scroll_offset; // count how much  up scrolled
} Map;
void initMap(Map *map, int width, int height); // staring point
void scrollMap(Map *map);                      // makes the map moving byremobing bottom row and adding top row
void drawMap(Map *map);                        // prints the map
void freeMap(Map *map);                        // clean up or avoide memory leaks
void generateRow(char *row, int width);        // puts random obstacles in the map

#endif // !MAP_H
       