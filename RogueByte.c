#include <stdio.h>   // for standard input-output{printf, fprintf, fscanf, fread, fwrite, fclose, fopen, fflush, perror}
#include <stdlib.h>  // for randomization, runing os commands, quit programs with code{ abs, rand, srand, exit, system}
#include <time.h>    // for generating random number with real time (time)
#include <stdbool.h> // for boolean function
#include <string.h>  // fro string functions {strlen, strncpy, strcmp, strrchr, strcspn, memcopy}
#include <ctype.h>   // for Character handling

// Platform-specific headers
#ifdef _WIN32
#include <conio.h>   // Including standard and platform-specific libraries
#include <windows.h> // Including standard and platform-specific libraries
#include <direct.h>  // Including standard and platform-specific libraries
// #define getch _getch
#else
#include <termios.h>   // for controlling terminal I/O behavior (input/output settings)
#include <unistd.h>    // program to talk directly to the OS (files, processes, environment) in Unix/Linux
#include <sys/ioctl.h> // for low-level device control like getting terminal size, modes
#include <sys/stat.h>  // for file and directory information & management.
#endif

// Game constants
#define MAP_WIDTH 40              // Game constant definition
#define MAP_HEIGHT 12             // Game constant definition
#define MAX_ENEMIES 25            // Game constant definition
#define MAX_LEADERBOARD 10        // Game constant definition
#define MSG_LINE_1 MAP_HEIGHT + 2 // Game constant definition
#define MSG_LINE_2 MAP_HEIGHT + 3 // Game constant definition
#define MSG_LINE_3 MAP_HEIGHT + 4 // Game constant definition

// Game state enumeration
typedef enum
{
    MAIN_MENU,
    IN_GAME,
    GAME_OVER,
    LEADERBOARD,
    NAME_ENTRY
} GameState;
// Different states of the game (menu, playing, game over, etc.)

// Player structure
typedef struct
{
    int x, y;
    int hp, max_hp;
    int strength;
    int level;
    int xp;
    int xp_to_level;
    int score;
    char name[50];
} Player;

// Enemy structure
typedef struct
{
    int x, y;
    int hp;
    int strength;
    int xp_value;
    bool is_boss;
} Enemy;

// Leaderboard entry structure
typedef struct
{
    char name[50];
    int level;
    int distance;
} LeaderboardEntry;

// Game data structure for saving/loading
typedef struct
{
    Player player;
    Enemy enemies[MAX_ENEMIES];
    int enemy_count;
    char game_map[MAP_HEIGHT][MAP_WIDTH];
    int world_offset;
    int move_count;
} GameData;

// Global variables
char game_map[MAP_HEIGHT][MAP_WIDTH];
Player player;
Enemy enemies[MAX_ENEMIES];
int enemy_count = 0;
LeaderboardEntry leaderboard[MAX_LEADERBOARD];
int leaderboard_size = 0;
int world_offset = 0;
int move_count = 0;
int initial_rows = MAP_HEIGHT / 2;

// Function prototypes

// Terminal control functions
void clear_screen();            // Function definition
void move_cursor(int x, int y); // Function definition
char getch();                   // Function definition
void enable_ansi();             // Function definition
void msleep(int milliseconds);  // Function definition

// Message system functions
void clear_messages();                                           // Function definition
void display_message(const char *msg, int line, bool important); // Function definition

// File path functions
char *get_leaderboard_path();
char *get_save_file_path();
void ensure_directory_exists(const char *path);             // Function definition
bool safe_rename(const char *oldpath, const char *newpath); // Function definition

// Game initialization functions
void init_player();           // Function definition
void generate_new_row(int y); // Function definition
void init_map();              // Function definition

// Game logic functions
void update_score();     // Function definition
void shift_world_down(); // Function definition
void spawn_enemies();    // Function definition
void move_enemies();     // Function definition
void check_collisions(); // Function definition

// Display functions
void draw_game();                  // Function definition
void show_welcome_screen();        // Function definition
int show_main_menu(bool has_save); // Function definition
void show_leaderboard();           // Function definition

// Leaderboard functions
void load_leaderboard();           // Function definition
bool update_leaderboard_entries(); // Function definition
void add_to_leaderboard();         // Function definition
void show_leaderboard();           // Function definition

// Save/load functions
bool save_game(const GameData *data); // Function definition
bool load_game(GameData *data);       // Function definition
bool save_file_exists();              // Function definition

// Game flow functions
void game_over();                     // Function definition
void get_player_name();               // Function definition
void handle_movement(int dx, int dy); // Function definition
void game_loop();                     // Function definition
// debugging
//void debug_game_state(); // Function definition

// Terminal control implementations
void clear_screen() // Function definition
{
#ifdef _WIN32
    system("cls");
#else
    printf("\033[2J\033[H");
#endif
}

void move_cursor(int x, int y) // Function definition
{
#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD pos = {x, y};
    SetConsoleCursorPosition(hConsole, pos);
#else
    printf("\033[%d;%dH", y + 1, x + 1);
#endif
}

#ifndef _WIN32
char getch() // Function definition
{
    struct termios oldt, newt;
    char ch;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
}
#endif


#ifdef _WIN32
void enable_ansi() // Function definition
{
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE)
        return;

    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode))
        return;

#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004 // Game constant definition
#endif

    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
}
#endif

void msleep(int milliseconds) // Function definition
{
    #ifdef _WIN32
        Sleep(milliseconds);
    #else
        usleep(milliseconds * 1000);
    #endif
}

// Message system implementations
void clear_messages() // Function definition
{
    for (int i = MSG_LINE_1; i <= MSG_LINE_3; i++)
    {
        move_cursor(0, i);
        printf("%-60s", "");
    }
    fflush(stdout);
}

void display_message(const char *msg, int line, bool important) // Function definition
{
    move_cursor(0, line);
    if (important)
    {
#ifdef _WIN32
        printf(">>> %-60s <<<", msg);
#else
        printf("\033[1;31m>>> %s <<<\033[0m", msg);
#endif
    }
    else
    {
        printf("%s", msg);
    }
    fflush(stdout);
}

// File path implementations
char *get_leaderboard_path()
{
    static char path[256];
    snprintf(path, sizeof(path), "leaderboard.txt"); // Force current directory
    // printf("DEBUG: Leaderboard path: %s\n", path);
    return path;
}

char *get_save_file_path()
{
    static char path[256];
    snprintf(path, sizeof(path), "savegame.dat"); // Force current directory
    // printf("DEBUG: Save file path: %s\n", path);
    return path;
}

void ensure_directory_exists(const char *path)
{ // Function definition
#ifndef _WIN32
    // For Unix: Check if directory exists
    char *sep = strrchr(path, '/');
    if (sep)
    {
        char dirpath[256];
        strncpy(dirpath, path, sep - path);
        dirpath[sep - path] = '\0';
        mkdir(dirpath, S_IRWXU); // Create if doesn't exist
    }
#endif
}

bool safe_rename(const char *oldpath, const char *newpath)
{ // Function definition
#ifdef _WIN32
    // Windows needs special handling
    remove(newpath); // Delete destination if exists
#endif
    return rename(oldpath, newpath) == 0; // Function definition
}

// Game initialization implementations
void init_player() // Function definition
{
    player.x = MAP_WIDTH / 2;
    player.y = initial_rows;
    player.max_hp = 20;
    player.hp = player.max_hp;
    player.strength = 5;
    player.level = 1;
    player.xp = 0;
    player.xp_to_level = 10;
    player.score = 0;
}

void generate_new_row(int y) // Function definitionww
{
    bool is_boss_room = (world_offset >= 200) && (world_offset % 200 == 0);

    for (int x = 0; x < MAP_WIDTH; x++)
    {
        if (x == 0 || x == MAP_WIDTH - 1)
        {
            game_map[y][x] = '|'; // Walls
        }
        else
        {
            if (is_boss_room && x >= MAP_WIDTH / 2 - 5 && x <= MAP_WIDTH / 2 + 5)
            {
                game_map[y][x] = (y == MAP_HEIGHT / 2) ? 'B' : '_';
            }
            else
            {
                int r = rand() % 100;
                if (r < 5)
                {
                    if (x < MAP_WIDTH - 2)
                    {
                        game_map[y][x] = '[';
                        game_map[y][x + 1] = ']';
                        x++;
                    }
                    else
                    {
                        game_map[y][x] = '_';
                    }
                }
                else if (r < 10) // Function definition
                {
                    game_map[y][x] = '~';
                }
                else
                {
                    game_map[y][x] = '_';
                }
            }
        }
    }
}

void init_map() // Function definition
{
    for (int y = 0; y < MAP_HEIGHT; y++)
    {
        generate_new_row(y);
    }
}

// Game logic implementations
void update_score() // Function definition
{
    player.score = world_offset;
}

void shift_world_down() // Function definition
{
    bool is_boss_room = (world_offset >= 200) && (world_offset % 200 == 0);

    if (is_boss_room)
    {
        int boss_x = MAP_WIDTH / 2;
        int boss_y = MAP_HEIGHT / 2;

        enemies[enemy_count] = (Enemy){
            boss_x, boss_y,
            40 + (world_offset / 30),
            8 + (world_offset / 60),
            80 + (world_offset / 25),
            true};
        enemy_count++;

        move_cursor(0, MSG_LINE_1);
        printf("\033[1;31m!!! BOSS AHEAD !!!\033[0m");
        move_cursor(0, MSG_LINE_2);
        printf("\033[1;31mDefeat it to progress!\033[0m");

        // Ensure messages stay visible for at least 2 moves
        draw_game();
        msleep(1000); // Brief pause but not blocking
    }

    for (int y = MAP_HEIGHT - 1; y > 0; y--)
    {
        memcpy(game_map[y], game_map[y - 1], MAP_WIDTH);
    }
    generate_new_row(0);
    world_offset++;
    update_score();

    player.y++;
    for (int i = 0; i < enemy_count; i++)
    {
        enemies[i].y++;
        if (enemies[i].y >= MAP_HEIGHT)
        {
            for (int j = i; j < enemy_count - 1; j++)
            {
                enemies[j] = enemies[j + 1];
            }
            enemy_count--;
            i--;
        }
    }
}

void spawn_enemies() // Function definition
{
    bool boss_alive = false;
    for (int i = 0; i < enemy_count; i++)
    {
        if (enemies[i].is_boss)
        {
            boss_alive = true;
            break;
        }
    }

    if (boss_alive || enemy_count >= MAX_ENEMIES)
        return;

    float progress_factor = 1 + (world_offset / 80.0f);
    int enemies_to_spawn = 3 + rand() % 3;

    for (int i = 0; i < enemies_to_spawn && enemy_count < MAX_ENEMIES; i++)
    {
        int x, y;
        do
        {
            x = 1 + rand() % (MAP_WIDTH - 2);
            y = 1 + rand() % (MAP_HEIGHT - 2);
        } while (game_map[y][x] != '_' ||
                 abs(x - player.x) < 5 ||
                 abs(y - player.y) < 5);

        enemies[enemy_count] = (Enemy){
            x, y,
            (int)(10 * progress_factor),
            (int)(4 * progress_factor),
            (int)(5 * progress_factor),
            false};
        enemy_count++;
    }
}

void move_enemies() // Function definition
{
    for (int i = 0; i < enemy_count; i++)
    {
        int dx = player.x - enemies[i].x;
        int dy = player.y - enemies[i].y;

        if (abs(dx) <= 5 && abs(dy) <= 5)
        {
            if (abs(dx) > abs(dy))
            {
                if (dx > 0 && game_map[enemies[i].y][enemies[i].x + 1] == '_')
                    enemies[i].x++;
                else if (dx < 0 && game_map[enemies[i].y][enemies[i].x - 1] == '_') // Function definition
                    enemies[i].x--;
            }
            else
            {
                if (dy > 0 && game_map[enemies[i].y + 1][enemies[i].x] == '_')
                    enemies[i].y++;
                else if (dy < 0 && game_map[enemies[i].y - 1][enemies[i].x] == '_') // Function definition
                    enemies[i].y--;
            }
        }
        else
        {
            int dir = rand() % 4;
            switch (dir)
            {
            case 0:
                if (game_map[enemies[i].y - 1][enemies[i].x] == '_')
                    enemies[i].y--;
                break;
            case 1:
                if (game_map[enemies[i].y + 1][enemies[i].x] == '_')
                    enemies[i].y++;
                break;
            case 2:
                if (game_map[enemies[i].y][enemies[i].x - 1] == '_')
                    enemies[i].x--;
                break;
            case 3:
                if (game_map[enemies[i].y][enemies[i].x + 1] == '_')
                    enemies[i].x++;
                break;
            }
        }
    }
}

void check_collisions() // Function definition
{
    for (int i = 0; i < enemy_count; i++)
    {
        if (player.x == enemies[i].x && player.y == enemies[i].y)
        {
            // Player attacks enemy
            enemies[i].hp -= player.strength;

            if (enemies[i].hp <= 0)
            {
                player.xp += enemies[i].xp_value;

                if (enemies[i].is_boss)
                {
                    clear_messages();
                    display_message("VICTORY! Boss defeated!", MSG_LINE_1, true);
                    display_message("You feel stronger!", MSG_LINE_2, true);
                    player.strength += 5;
                    draw_game();
                    msleep(3500);
                }

                // Remove defeated enemy
                for (int j = i; j < enemy_count - 1; j++)
                {
                    enemies[j] = enemies[j + 1];
                }
                enemy_count--;
                i--;

                // Check for level up
                if (player.xp >= player.xp_to_level)
                {
                    player.level++;
                    player.xp -= player.xp_to_level;
                    player.xp_to_level = (int)(player.xp_to_level * 1.5);
                    player.max_hp += 5;
                    player.hp = player.max_hp;
                    player.strength += 2;
                }
            }
            else
            {
                // Enemy attacks player
                player.hp -= enemies[i].strength;

                // Immediate death check and handling
                if (player.hp <= 0)
                {
                    player.hp = 0; // Prevent negative HP
                    return;        // Exit immediately
                }
            }
            break;
        }
    }
}

// Display implementations
// Modified draw_game() function with better player stats display
void draw_game() // Function definition
{
    clear_screen();
    bool is_boss_room = (world_offset >= 200) && (world_offset % 200 == 0);

    // Draw map
    for (int y = 0; y < MAP_HEIGHT; y++)
    {
        for (int x = 0; x < MAP_WIDTH; x++)
        {
            move_cursor(x, y);
            if (is_boss_room && x >= MAP_WIDTH / 2 - 5 && x <= MAP_WIDTH / 2 + 5)
            {
                printf("\033[48;5;52m");
            }
            if (game_map[y][x] == '~')
            {
                printf("\033[0;36m~\033[0m");
            }
            else
            {
                putchar(game_map[y][x]);
            }
            printf("\033[0m");
        }
    }

    // Draw player
    move_cursor(player.x, player.y);
    printf("\033[1;37m@\033[0m");

    // Draw enemies
    for (int i = 0; i < enemy_count; i++)
    {
        move_cursor(enemies[i].x, enemies[i].y);
        if (enemies[i].is_boss)
        {
            printf("\033[1;33mB\033[0m");
        }
        else
        {
            printf("\033[0;91me\033[0m");
        }
    }

    // Enhanced player stats display
    move_cursor(0, MAP_HEIGHT);
    printf("\033[1;36mPlayer: %s\033[0m", player.name);

    move_cursor(0, MAP_HEIGHT + 1);
    printf("\033[1;33mHP: %d/%d | STR: %d | LVL: %d | XP: %d/%d | Score: %d\033[0m",
           player.hp, player.max_hp, player.strength, player.level,
           player.xp, player.xp_to_level, player.score);

    move_cursor(0, MAP_HEIGHT + 2);
    printf("\033[1;37mControls: WASD to move, P to save, Q to quit\033[0m");

    // Nearby enemies display
    int stat_line = MAP_HEIGHT + 3;
    move_cursor(0, stat_line);
    printf("\033[1;31mNearby enemies: \033[0m");

    int visible_count = 0;
    for (int i = 0; i < enemy_count && visible_count < 2; i++)
    {
        if (abs(enemies[i].x - player.x) <= 3 &&
            abs(enemies[i].y - player.y) <= 3)
        {
            move_cursor(0, stat_line + 1 + visible_count);
            printf("%s \033[1;31mHP:\033[0m%-3d \033[1;31mSTR:\033[0m%-2d",
                   enemies[i].is_boss ? "\033[1;33mBOSS\033[0m" : "\033[0;91mEnemy\033[0m",
                   enemies[i].hp,
                   enemies[i].strength);
            visible_count++;
        }
    }

    for (int i = visible_count; i < 2; i++)
    {
        move_cursor(0, stat_line + 1 + i);
        printf("                ");
    }

    if (visible_count == 0)
    {
        move_cursor(16, stat_line);
        printf("\033[0;37mNone\033[0m");
    }

    fflush(stdout);
}

void show_welcome_screen() // Function definition
{
    clear_screen();
    move_cursor(MAP_WIDTH / 2 - 10, MAP_HEIGHT / 2 - 2);
    printf("\033[1;35mWelcome to RougeByte!\033[0m");
    move_cursor(MAP_WIDTH / 2 - 10, MAP_HEIGHT / 2 - 1);
    printf("\033[0;36mBeat your best steps!\033[0m");
    fflush(stdout);
    msleep(3000);
}

int show_main_menu(bool has_save) // Function definition
{
    clear_screen();
    int selected = 0;
    int option_count = has_save ? 4 : 3; // 4 options if save exists, 3 otherwise
    char options[4][20] = {
        "Continue",
        "New Game",
        "Leaderboard",
        "Exit"};

    while (1)
    {
        clear_screen();
        move_cursor(MAP_WIDTH / 2 - 8, MAP_HEIGHT / 2 - 3);
        printf("\033[1;34mMAIN MENU\033[0m");

        // Only show Continue if save exists
        int start_index = has_save ? 0 : 1;

        for (int i = 0; i < option_count; i++)
        {
            move_cursor(MAP_WIDTH / 2 - 8, MAP_HEIGHT / 2 - 1 + i);
            if (i == selected)
            {
                printf("\033[1;32m> %s\033[0m", options[start_index + i]);
            }
            else
            {
                printf("  %s", options[start_index + i]);
            }
        }

        char ch = getch();
        if (ch == 'w' || ch == 'W')
        {
            selected = (selected - 1 + option_count) % option_count;
        }
        else if (ch == 's' || ch == 'S') 
        {
            selected = (selected + 1) % option_count;
        }
        else if (ch == '\r' || ch == '\n') 
        {
            return has_save ? selected : selected + 1; // Adjust return value
        }
    }
}
// Leaderboard implementations
void load_leaderboard() // Function definition
{
    FILE *file = fopen(get_leaderboard_path(), "r");
    if (!file)
    {
        leaderboard_size = 0;
        return;
    }

    leaderboard_size = 0;
    while (fscanf(file, "%50s %d %d",
                  leaderboard[leaderboard_size].name,
                  &leaderboard[leaderboard_size].level,
                  &leaderboard[leaderboard_size].distance) == 3 &&
           leaderboard_size < MAX_LEADERBOARD)
    {
        leaderboard_size++;
    }
    fclose(file);
}

bool update_leaderboard_entries() // Function definition
{
    for (int i = 0; i < leaderboard_size; i++)
    {
        if (strlen(leaderboard[i].name) == 0 ||
            leaderboard[i].level <= 0 ||
            leaderboard[i].distance < 0)
        {
            return false;
        }
    }

    bool exists = false;
    for (int i = 0; i < leaderboard_size; i++)
    {
        if (strcmp(leaderboard[i].name, player.name) == 0)
        {
            if (player.score > leaderboard[i].distance)
            {
                leaderboard[i].level = player.level;
                leaderboard[i].distance = player.score;
            }
            exists = true;
            break;
        }
    }

    if (!exists)
    {
        if (leaderboard_size < MAX_LEADERBOARD)
        {
            strncpy(leaderboard[leaderboard_size].name, player.name, 4);
            leaderboard[leaderboard_size].level = player.level;
            leaderboard[leaderboard_size].distance = player.score;
            leaderboard_size++;
        }
        else
        {
            int lowest_index = 0;
            for (int i = 1; i < leaderboard_size; i++)
            {
                if (leaderboard[i].distance < leaderboard[lowest_index].distance)
                {
                    lowest_index = i;
                }
            }

            if (player.score > leaderboard[lowest_index].distance)
            {
                strncpy(leaderboard[lowest_index].name, player.name, 50);
                leaderboard[lowest_index].level = player.level;
                leaderboard[lowest_index].distance = player.score;
            }
        }
    }

    // Sort leaderboard
    for (int i = 0; i < leaderboard_size - 1; i++)
    {
        for (int j = 0; j < leaderboard_size - i - 1; j++)
        {
            if (leaderboard[j].distance < leaderboard[j + 1].distance ||
                (leaderboard[j].distance == leaderboard[j + 1].distance &&
                 leaderboard[j].level < leaderboard[j + 1].level))
            {
                LeaderboardEntry temp = leaderboard[j];
                leaderboard[j] = leaderboard[j + 1];
                leaderboard[j + 1] = temp;
            }
        }
    }
    return true;
}

void add_to_leaderboard()
{ // Function definition
    // printf("\nDEBUG: Attempting to add %s with score %d\n", player.name, player.score);

    // Load current leaderboard first
    load_leaderboard();

    // Check if player already exists
    bool exists = false;
    for (int i = 0; i < leaderboard_size; i++)
    {
        if (strcmp(leaderboard[i].name, player.name) == 0)
        {
            if (player.score > leaderboard[i].distance)
            {
                leaderboard[i].distance = player.score;
                leaderboard[i].level = player.level;
                // printf("DEBUG: Updated existing entry\n");
            }
            exists = true;
            break;
        }
    }

    // Add new entry if needed
    if (!exists)
    {
        if (leaderboard_size < MAX_LEADERBOARD)
        {
            strncpy(leaderboard[leaderboard_size].name, player.name, 49);
            leaderboard[leaderboard_size].level = player.level;
            leaderboard[leaderboard_size].distance = player.score;
            leaderboard_size++;
            // printf("DEBUG: Added new entry\n");
        }
        else
        {
            // Find lowest score to replace
            int lowest_index = 0;
            for (int i = 1; i < leaderboard_size; i++)
            {
                if (leaderboard[i].distance < leaderboard[lowest_index].distance)
                {
                    lowest_index = i;
                }
            }
            if (player.score > leaderboard[lowest_index].distance)
            {
                strncpy(leaderboard[lowest_index].name, player.name, 49);
                leaderboard[lowest_index].level = player.level;
                leaderboard[lowest_index].distance = player.score;
                // printf("DEBUG: Replaced lowest entry\n");
            }
        }
    }

    // Sort the leaderboard
    for (int i = 0; i < leaderboard_size - 1; i++)
    {
        for (int j = 0; j < leaderboard_size - i - 1; j++)
        {
            if (leaderboard[j].distance < leaderboard[j + 1].distance ||
                (leaderboard[j].distance == leaderboard[j + 1].distance &&
                 leaderboard[j].level < leaderboard[j + 1].level))
            {
                LeaderboardEntry temp = leaderboard[j];
                leaderboard[j] = leaderboard[j + 1];
                leaderboard[j + 1] = temp;
            }
        }
    }

    // Save to file
    char *path = get_leaderboard_path();
    FILE *file = fopen(path, "w");
    if (file)
    {
        for (int i = 0; i < leaderboard_size; i++)
        {
            fprintf(file, "%s %d %d\n",
                    leaderboard[i].name,
                    leaderboard[i].level,
                    leaderboard[i].distance);
        }
        fclose(file);
        // printf("DEBUG: Leaderboard saved to %s\n", path);
    }
    else
    {
        // printf("ERROR: Could not save leaderboard!\n");
    }
}

void show_leaderboard()
{ // Function definition
    int selected = 0;

    while (1)
    {
        clear_screen();

        // Header with color
        move_cursor(0, 0);
        printf("\033[1;36m=== LEADERBOARD ===\033[0m"); // Cyan header
        move_cursor(0, 1);
        printf("\033[1;94mRank  Name           Level  Distance\033[0m"); // Yellow column headers

        // Leaderboard entries
        for (int i = 0; i < leaderboard_size; i++)
        {
            move_cursor(0, 2 + i);

            // Apply medal-based color styling
            if (i == 0)
                printf("\033[1;93m"); // Gold (bright yellow)
            else if (i == 1)          // Function definition
                printf("\033[1;97m"); // Silver (bright white)
            else if (i == 2)          // Function definition
                printf("\033[1;30m"); // Bronze (regular yellow)
            else
                printf("\033[0;250m"); // Regular white/gray

            printf("%2d.   %-12s   %3d     %5d\033[0m",
                   i + 1,
                   leaderboard[i].name,
                   leaderboard[i].level,
                   leaderboard[i].distance);
        }

        // Menu options - positioned below with colors
        int menu_pos = 3 + leaderboard_size;
        move_cursor(0, menu_pos);
        printf("\n"); // Spacer

        // Return to Menu option
        move_cursor(0, menu_pos + 1);
        if (selected == 0)
        {
            printf("\033[1;92m> "); // Bright green for selection
        }
        else
        {
            printf("\033[0;37m  "); // Normal white
        }
        printf("Return to Menu\033[0m");

        // Exit Game option
        move_cursor(0, menu_pos + 2);
        if (selected == 1)
        {
            printf("\033[1;32m> ");
        }
        else
        {
            printf("\033[0;37m  ");
        }
        printf("Exit Game\033[0m");

        // Input handling
        char ch = getch();
        switch (tolower(ch))
        {
        case 'w':
            selected = (selected - 1 + 2) % 2;
            break;
        case 's':
            selected = (selected + 1) % 2;
            break;
        case '\r': // Enter
        case '\n':
            if (selected == 0)
            {
                printf("\033[0m"); // Reset colors before returning
                return;
            }
            else
            {
                exit(0);
            }
            break;
        }

        fflush(stdout); // Ensure all output is displayed
    }
}
// Save/load implementations

bool save_game(const GameData *data)
{ // Function definition
    if (data->player.hp <= 0)
    {
        // printf("DEBUG: Not saving - player is dead\n");
        return false;
    }

    char *path = get_save_file_path();
    // printf("DEBUG: Attempting to save to: %s\n", path);

    FILE *file = fopen(path, "wb");
    if (!file)
    {
        // printf("DEBUG: Failed to open save file\n");
        // perror("Error");
        return false;
    }

    bool success = fwrite(data, sizeof(GameData), 1, file) == 1;
    fclose(file);

    if (success)
    {
        // printf("DEBUG: Game saved successfully\n");
        //  Verify the file exists
        file = fopen(path, "rb");
        // printf("DEBUG: Save file verification: %s\n", file ? "SUCCESS" : "FAILED");
        if (file)
            fclose(file);
    }
    else
    {
        // printf("DEBUG: Failed to write save data\n");
    }

    return success;
}

bool load_game(GameData *data)
{ // Function definition
    char *path = get_save_file_path();
    FILE *file = fopen(path, "rb");
    if (!file)
    {
        return false;
    }

    bool success = fread(data, sizeof(GameData), 1, file) == 1;
    fclose(file);

    if (success && data->player.hp <= 0)
    {
        remove(path);
        return false;
    }
    return success;
}

bool save_file_exists()
{ // Function definition
    FILE *file = fopen(get_save_file_path(), "rb");
    if (file)
    {
        // Verify it contains valid data
        GameData test;
        bool valid = fread(&test, sizeof(GameData), 1, file) == 1;
        fclose(file);
        return valid;
    }
    return false;
}

// Game flow implementations
void game_over() // Function definition
{
    clear_screen();
#ifdef _WIN32
    move_cursor(MAP_WIDTH / 2 - 15, MAP_HEIGHT / 2 - 1);
    printf("================================");
    move_cursor(MAP_WIDTH / 2 - 15, MAP_HEIGHT / 2);
    printf("        GAME OVER!             ");
    move_cursor(MAP_WIDTH / 2 - 15, MAP_HEIGHT / 2 + 1);
    printf("  Final Score: %-10d      ", player.score);
    move_cursor(MAP_WIDTH / 2 - 15, MAP_HEIGHT / 2 + 2);
    printf("================================");
#else
    move_cursor(MAP_WIDTH / 2 - 15, MAP_HEIGHT / 2 - 1);
    printf("\033[1;31m╔══════════════════════════╗\033[0m");
    move_cursor(MAP_WIDTH / 2 - 15, MAP_HEIGHT / 2);
    printf("\033[1;31m║      GAME OVER!          ║\033[0m");
    move_cursor(MAP_WIDTH / 2 - 15, MAP_HEIGHT / 2 + 1);
    printf("\033[1;31m║ Final Score: %-10d  ║\033[0m", player.score);
    move_cursor(MAP_WIDTH / 2 - 15, MAP_HEIGHT / 2 + 2);
    printf("\033[1;31m╚══════════════════════════╝\033[0m");
#endif
    fflush(stdout);

    add_to_leaderboard();

    // Clean up save file
    char *save_path = get_save_file_path();
    remove(save_path);

    msleep(3000);
}

void get_player_name() // Function definition
{
#ifndef _WIN32
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag |= ECHO | ICANON;
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
#endif

    clear_screen();
    printf("Enter your name (max 50 chars): ");
    fflush(stdout);

    char input[51];
    if (fgets(input, 51, stdin))
    {
        input[strcspn(input, "\n")] = '\0';
        if (strlen(input) > 0)
        {
            strncpy(player.name, input, 50);
            player.name[50] = '\0';
        }
    }

#ifndef _WIN32
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
#endif
}

void handle_movement(int dx, int dy) // Function definition
{
    bool boss_alive = false;
    for (int i = 0; i < enemy_count; i++)
    {
        if (enemies[i].is_boss)
        {
            boss_alive = true;
            break;
        }
    }

    int new_x = player.x + dx;
    int new_y = player.y + dy;

    if (new_x < 0 || new_x >= MAP_WIDTH || new_y < 0 || new_y >= MAP_HEIGHT)
        return;

    if (game_map[new_y][new_x] == '_')
    {
        player.x = new_x;
        player.y = new_y;

        // Only shift world if not in boss room or boss is dead
        if (dy < 0 && (!boss_alive || !((world_offset > 0) && (world_offset % 100 == 0))))
        {
            update_score();
            if (player.y < MAP_HEIGHT / 4)
            {
                shift_world_down();
            }
        }

        move_enemies();
        check_collisions();

        if (++move_count % 20 == 0)
        {
            spawn_enemies();
        }
    }
}

void game_loop() // Function definition
{
    GameState state = MAIN_MENU;
    // Different states of the game (menu, playing, game over, etc.)
    GameData game_data;
    bool has_save = save_file_exists();

    show_welcome_screen();
    load_leaderboard();

    while (1)
    {
        // Always check for save file when returning to menu
        if (state == MAIN_MENU)
        {
            has_save = save_file_exists();
        }

        switch (state)
        {
        case MAIN_MENU:
        {
            int choice = show_main_menu(has_save);
            if (choice == 0 && has_save)
            { // Continue
                if (load_game(&game_data))
                {
                    // Copy loaded data to game state
                    player = game_data.player;
                    memcpy(enemies, game_data.enemies, sizeof(enemies));
                    enemy_count = game_data.enemy_count;
                    memcpy(game_map, game_data.game_map, sizeof(game_map));
                    world_offset = game_data.world_offset;
                    move_count = game_data.move_count;
                    state = IN_GAME;
                }
            }
            else if (choice == 1) // Function definition
            {                     // New Game
                // Delete any existing save file when starting new game
                char *save_path = get_save_file_path();
                remove(save_path);

                get_player_name();
                init_player();
                init_map();
                enemy_count = 0;
                world_offset = 0;
                move_count = 0;
                spawn_enemies();
                state = IN_GAME;
            }
            else if (choice == 2) // Function definition
            {                     // Leaderboard
                state = LEADERBOARD;
            }
            else
            { // Exit
                return;
            }
            break;
        }

        case IN_GAME:
        {
            // Check for death before processing anything else
            if (player.hp <= 0)
            {
                state = GAME_OVER;
                break;
            }

            draw_game();
            char ch = tolower(getch());

            if (ch == 'p')
            { // Save game
                GameData save;
                save.player = player;
                memcpy(save.enemies, enemies, sizeof(enemies));
                save.enemy_count = enemy_count;
                memcpy(save.game_map, game_map, sizeof(game_map));
                save.world_offset = world_offset;
                save.move_count = move_count;

                if (save_game(&save))
                {
                    display_message("Game saved!", MSG_LINE_1, true);
                    draw_game();
                    state = MAIN_MENU;
                }
                else
                {
                    display_message("Save failed!", MSG_LINE_1, true);
                    draw_game();
                }
            }
            else if (ch == 'q') // Function definition
            {                   // Quit to menu
                state = MAIN_MENU;
            }
            else
            { // Handle movement
                switch (ch)
                {
                case 'w':
                    handle_movement(0, -1);
                    break;
                case 'a':
                    handle_movement(-1, 0);
                    break;
                case 's':
                    handle_movement(0, 1);
                    break;
                case 'd':
                    handle_movement(1, 0);
                    break;
                }

                // Process enemy movement and collisions after player moves
                move_enemies();
                check_collisions();

                // Spawn new enemies periodically
                if (++move_count % 20 == 0)
                {
                    spawn_enemies();
                }
            }
            break;
        }

        case GAME_OVER:
        {
            game_over();
            state = LEADERBOARD;
            break;
        }

        case LEADERBOARD:
        {
            show_leaderboard();
            state = MAIN_MENU;
            break;
        }
        }
    }
}

// dbugging
//  void debug_game_state() {
//      printf("\nDEBUG: Game State\n");
//      printf("Player: %s (HP: %d/%d)\n", player.name, player.hp, player.max_hp);
//      printf("Score: %d, Level: %d\n", player.score, player.level);
//      printf("Save exists: %s\n", save_file_exists() ? "YES" : "NO");

//     FILE *lb = fopen(get_leaderboard_path(), "r");
//     printf("Leaderboard exists: %s\n", lb ? "YES" : "NO");
//     if (lb) fclose(lb);
// }
int main() // Main function: entry point of the game
{
    srand(time(NULL));

#ifdef _WIN32
    enable_ansi();
#endif

    game_loop();
    return 0;
}