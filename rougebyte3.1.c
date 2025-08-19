#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

// Platform-specific headers
#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#define getch _getch
#else
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#endif

#define MAP_WIDTH 40
#define MAP_HEIGHT 12
#define MAX_ENEMIES 25
#define MAX_LEADERBOARD 10
#define MSG_LINE_1 MAP_HEIGHT + 2
#define MSG_LINE_2 MAP_HEIGHT + 3
#define MSG_LINE_3 MAP_HEIGHT + 4

// terminal control functions

void clear_screen();
void move_cursor(int x, int y);
char getch();       // Linux/macOS version
void enable_ansi(); // Windows-specific
void msleep(int milliseconds);

// messege system functions

void clear_messages();
void display_message(const char *msg, int line, bool important);

// File Path Function

char *get_leaderboard_path();

// Game Initialization Functions

void init_player();
void generate_new_row(int y);
void init_map();

// Game Logic Functions

void update_score();
void shift_world_down();
void spawn_enemies();
void move_enemies();
void check_collisions();

// Display Function

void draw_game();

// Leaderboard Functions

void load_leaderboard();
bool update_leaderboard_entries();
void add_to_leaderboard();
void show_leaderboard();
void game_over();

// Player Input Functions

void get_player_name();
void handle_movement(int dx, int dy);

// Cross-platform terminal control
void clear_screen()
{
#ifdef _WIN32
    system("cls");
#else
    printf("\033[2J\033[H");
#endif
}

void move_cursor(int x, int y)
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
char getch()
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
void enable_ansi()
{
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE)
        return;

    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode))
        return;

#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif

    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
}
#endif

void msleep(int milliseconds)
{
#ifdef _WIN32
    Sleep(milliseconds);
#else
    usleep(milliseconds * 1000);
#endif
}

// Message system
void clear_messages()
{
    for (int i = MSG_LINE_1; i <= MSG_LINE_3; i++)
    {
        move_cursor(0, i);
        printf("%-60s", "");
    }
    fflush(stdout);
}

void display_message(const char *msg, int line, bool important)
{
    move_cursor(0, line);
    if (important)
    {
#ifdef _WIN32
        printf(">>> %s <<<", msg);
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

typedef struct
{
    int x, y;
    int hp;
    int strength;
    int xp_value;
    bool is_boss;
} Enemy;

typedef struct
{
    char name[50];
    int level;
    int distance;
} LeaderboardEntry;

char game_map[MAP_HEIGHT][MAP_WIDTH];
Player player;
Enemy enemies[MAX_ENEMIES];
int enemy_count = 0;
LeaderboardEntry leaderboard[MAX_LEADERBOARD];
int leaderboard_size = 0;
int world_offset = 0;
int move_count = 0;
int initial_rows = MAP_HEIGHT / 2;

char *get_leaderboard_path()
{
    static char path[256];
#ifdef _WIN32
    snprintf(path, sizeof(path), "leaderboard.txt");
#else
    snprintf(path, sizeof(path), "%s/.rougebyte_leaderboard.txt", getenv("HOME"));
#endif
    return path;
}

void init_player()
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

void generate_new_row(int y)
{
    bool is_boss_room = (world_offset > 0) && (world_offset % 10 == 0);

    for (int x = 0; x < MAP_WIDTH; x++)
    {
        if (x == 0 || x == MAP_WIDTH - 1)
        {
            game_map[y][x] = '|'; // Walls
        }
        else
        {
            // Generate boss arena (center 10 tiles)
            if (is_boss_room && x >= MAP_WIDTH / 2 - 5 && x <= MAP_WIDTH / 2 + 5)
            {
                game_map[y][x] = (y == MAP_HEIGHT / 2) ? 'B' : '_'; // Boss spawns in middle row
            }
            // Normal terrain
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
                else if (r < 10)
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

void init_map()
{
    for (int y = 0; y < MAP_HEIGHT; y++)
    {
        generate_new_row(y);
    }
}

void update_score()
{
    player.score = world_offset;
}

void shift_world_down()
{
    // Check if current row is a boss room
    bool is_boss_room = (world_offset > 0) && (world_offset % 10 == 0);

    if (is_boss_room)
    {
        // Spawn boss in the center
        int boss_x = MAP_WIDTH / 2;
        int boss_y = MAP_HEIGHT / 2;

        enemies[enemy_count] = (Enemy){
            boss_x, boss_y,
            50 + (world_offset / 25),  // HP: +4 per 100 tiles (instead of +10)
            10 + (world_offset / 50),  // Damage: +2 per 100 tiles (instead of +5)
            100 + (world_offset / 20), // XP also scales slightly
            true};
        enemy_count++;

        // Pause game and show warning
        clear_messages();
        display_message("!!! WARNING: BOSS AHEAD !!!", MSG_LINE_1, true);
        display_message("The air grows heavy...", MSG_LINE_2, false);
        draw_game();
        msleep(3000); // 3-second delay
    }

    // Shift world as normal
    for (int y = MAP_HEIGHT - 1; y > 0; y--)
    {
        memcpy(game_map[y], game_map[y - 1], MAP_WIDTH);
    }
    generate_new_row(0);
    world_offset++;
    update_score();

    // Move player and enemies down
    player.y++;
    for (int i = 0; i < enemy_count; i++)
    {
        enemies[i].y++;
        if (enemies[i].y >= MAP_HEIGHT)
        {
            // Remove off-screen enemies
            for (int j = i; j < enemy_count - 1; j++)
            {
                enemies[j] = enemies[j + 1];
            }
            enemy_count--;
            i--;
        }
    }
}

void spawn_enemies()
{
    // Check if boss exists
    bool boss_alive = false;
    for (int i = 0; i < enemy_count; i++)
    {
        if (enemies[i].is_boss)
        {
            boss_alive = true;
            break;
        }
    }

    // Stop ALL spawns if boss is alive
    if (boss_alive || enemy_count >= MAX_ENEMIES)
        return;

    float progress_factor = 1 + (world_offset / 20.0f);

    // Spawn normal enemies (only if no boss exists)
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

void move_enemies()
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
                else if (dx < 0 && game_map[enemies[i].y][enemies[i].x - 1] == '_')
                    enemies[i].x--;
            }
            else
            {
                if (dy > 0 && game_map[enemies[i].y + 1][enemies[i].x] == '_')
                    enemies[i].y++;
                else if (dy < 0 && game_map[enemies[i].y - 1][enemies[i].x] == '_')
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

void check_collisions()
{
    for (int i = 0; i < enemy_count; i++)
    {
        if (player.x == enemies[i].x && player.y == enemies[i].y)
        {
            // Apply damage to enemy
            enemies[i].hp -= player.strength;

            if (enemies[i].hp <= 0)
            {
                // Enemy defeated
                player.xp += enemies[i].xp_value;

                if (enemies[i].is_boss)
                {
                    // BOSS DEFEATED - Show message and delay
                    clear_messages();
                    display_message("VICTORY! Boss defeated!", MSG_LINE_1, true);
                    display_message("You feel stronger!", MSG_LINE_2, true);
                    player.strength += 5;
                    draw_game();
                    msleep(2000); // 2-second pause
                }

                // Remove enemy
                for (int j = i; j < enemy_count - 1; j++)
                {
                    enemies[j] = enemies[j + 1];
                }
                enemy_count--;
                i--;

                // Silent level-up (no message/delay)
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
                // Enemy hit player (silent)
                player.hp -= enemies[i].strength;

                if (player.hp <= 0)
                {
                    game_over();
                }
            }
            break;
        }
    }
}

void draw_game()
{
    clear_screen();

    bool is_boss_room = (world_offset > 0) && (world_offset % 10 == 0);

    for (int y = 0; y < MAP_HEIGHT; y++)
    {
        for (int x = 0; x < MAP_WIDTH; x++)
        {
            move_cursor(x, y);

            // Boss room background (darker shade)
            if (is_boss_room)
            {
                printf("\033[48;5;88m"); // Darker red background
            }

            // Draw terrain
            if (game_map[y][x] == '~')
            {
                printf("\033[0;36m~\033[0m"); // Blue water
            }
            else
            {
                putchar(game_map[y][x]);
            }

            printf("\033[0m"); // Reset color
        }
    }

    // Draw player (now with white color for contrast)
    move_cursor(player.x, player.y);
    printf("\033[1;37m@\033[0m"); // Bright white player

    // Draw enemies
    for (int i = 0; i < enemy_count; i++)
    {
        move_cursor(enemies[i].x, enemies[i].y);
        if (enemies[i].is_boss)
        {
            printf("\033[1;33mB\033[0m"); // Changed boss to BRIGHT YELLOW
        }
        else
        {
            printf("\033[0;91me\033[0m"); // Regular enemies stay light red
        }
    }

    // Player stats UI
    move_cursor(0, MAP_HEIGHT);
    printf("HP: %d/%d | STR: %d | LVL: %d | XP: %d/%d | Score: %d",
           player.hp, player.max_hp, player.strength, player.level,
           player.xp, player.xp_to_level, player.score);
    move_cursor(0, MAP_HEIGHT + 1);
    printf("Controls: WASD to move, Q to quit, L for leaderboard");

    // Enemy stats panel - always show header with clear empty state
    int stat_line = MAP_HEIGHT + 2;
    move_cursor(0, stat_line);
    printf("Nearby enemies: ");

    int visible_count = 0;
    for (int i = 0; i < enemy_count && visible_count < 2; i++)
    {
        if (abs(enemies[i].x - player.x) <= 3 &&
            abs(enemies[i].y - player.y) <= 3)
        {
            move_cursor(0, stat_line + 1 + visible_count);
            printf("%s HP:%-3d STR:%-2d",
                   enemies[i].is_boss ? "BOSS " : "Enemy",
                   enemies[i].hp,
                   enemies[i].strength);
            visible_count++;
        }
    }

    // Clear any remaining lines
    for (int i = visible_count; i < 2; i++)
    {
        move_cursor(0, stat_line + 1 + i);
        printf("                "); // Clear exactly 16 spaces
    }

    // Show "None" when no enemies nearby
    if (visible_count == 0)
    {
        move_cursor(16, stat_line); // Right after "Nearby enemies: "
        printf("None");
    }

    fflush(stdout);
}
void load_leaderboard()
{
    FILE *file = fopen(get_leaderboard_path(), "r");
    if (file == NULL)
    {
        leaderboard_size = 0;
        return;
    }

    leaderboard_size = 0;
    while (fscanf(file, "%49s %d %d",
                  leaderboard[leaderboard_size].name,
                  &leaderboard[leaderboard_size].level,
                  &leaderboard[leaderboard_size].distance) == 3 &&
           leaderboard_size < MAX_LEADERBOARD)
    {
        leaderboard_size++;
    }

    fclose(file);
}

bool update_leaderboard_entries()
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
            strncpy(leaderboard[leaderboard_size].name, player.name, 49);
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
                strncpy(leaderboard[lowest_index].name, player.name, 49);
                leaderboard[lowest_index].level = player.level;
                leaderboard[lowest_index].distance = player.score;
            }
        }
    }

    for (int i = 0; i < leaderboard_size - 1; i++)
    {
        for (int j = 0; j < leaderboard_size - i - 1; j++)
        {
            if (leaderboard[j].distance < leaderboard[j + 1].distance)
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
{
    LeaderboardEntry backup[MAX_LEADERBOARD];
    memcpy(backup, leaderboard, sizeof(backup));
    int backup_size = leaderboard_size;

    if (update_leaderboard_entries())
    {
        char temp_path[256];
        snprintf(temp_path, sizeof(temp_path), "%s.tmp", get_leaderboard_path());

        FILE *file = fopen(temp_path, "w");
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

#ifdef _WIN32
            remove(get_leaderboard_path());
#endif
            rename(temp_path, get_leaderboard_path());
        }
        else
        {
            memcpy(leaderboard, backup, sizeof(leaderboard));
            leaderboard_size = backup_size;
        }
    }
}

void show_leaderboard()
{
    clear_screen();
    move_cursor(0, 0);
    printf("=== LEADERBOARD ===");
    move_cursor(0, 1);
    printf("Rank  Name           Level  Distance");

    for (int i = 0; i < leaderboard_size; i++)
    {
        move_cursor(0, 2 + i);
        printf("%2d.   %-12s   %3d     %5d",
               i + 1, leaderboard[i].name, leaderboard[i].level, leaderboard[i].distance);
    }

    move_cursor(0, MAP_HEIGHT);
    printf("Press any key to continue...\n");
    fflush(stdout);
    getch();
}

void game_over()
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
    msleep(2000); // Pause for 2 seconds

    add_to_leaderboard();
    show_leaderboard(); // Show leaderboard before exit
    getch();
    exit(0);
}

void get_player_name()
{
#ifndef _WIN32
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag |= ECHO | ICANON;
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
#endif

    clear_screen();
    printf("Enter your name (max 49 chars): ");
    fflush(stdout);

    char input[50];
    if (fgets(input, 50, stdin) == NULL)
    {
        strncpy(player.name, "Player", 49);
    }
    else
    {
        input[strcspn(input, "\n")] = '\0';
        if (strlen(input) == 0)
        {
            strncpy(player.name, "Player", 49);
        }
        else
        {
            strncpy(player.name, input, 49);
        }
    }
    player.name[49] = '\0';

#ifndef _WIN32
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
#endif
}

void handle_movement(int dx, int dy)
{
    // Check if a boss exists
    bool boss_alive = false;
    for (int i = 0; i < enemy_count; i++)
    {
        if (enemies[i].is_boss)
        {
            boss_alive = true;
            break;
        }
    }

    // Prevent moving up if boss is alive
    if (boss_alive && dy < 0)
    {
        display_message("Defeat the BOSS first!", MSG_LINE_1, true);
        return;
    }

    int new_x = player.x + dx;
    int new_y = player.y + dy;

    if (new_x < 0 || new_x >= MAP_WIDTH || new_y < 0 || new_y >= MAP_HEIGHT)
        return;

    if (game_map[new_y][new_x] == '_')
    {
        player.x = new_x;
        player.y = new_y;

        if (dy < 0)
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

int main()
{
    srand(time(NULL));

#ifdef _WIN32
    enable_ansi();
#endif

    init_player();
    get_player_name();
    load_leaderboard();

    init_map();
    spawn_enemies();

    bool running = true;
    while (running)
    {
        draw_game();

        char ch = tolower(getch());
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
        case 'q':
            add_to_leaderboard();
            show_leaderboard();
            running = false;
            break;
        case 'l':
            show_leaderboard();
            break;
        }
    }

    return 0;
}