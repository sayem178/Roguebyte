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

// Cross-platform terminal control
void clear_screen()
{
#ifdef _WIN32
    system("cls");
#else
    printf("\033[2J\033[H"); // ANIS escape code
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

#ifndef _WIN32 // for linus and macOS only
char getch()
{
    struct termios oldt, newt;               // a struct that controls terminal settings
    char ch;                                 // input character (WASD)
    tcgetattr(STDIN_FILENO, &oldt);          // Get the terminal settings of standard input (keyboard).
    newt = oldt;                             // Makes a copy of old settings into newt
    newt.c_lflag &= ~(ICANON | ECHO);        // Turn off line buffering and stop echoing typed keys on the terminal.
    tcsetattr(STDIN_FILENO, TCSANOW, &newt); // Apply the new settings immediately.
    ch = getchar();                          // Reads one character from input, without waiting for Enter and without showing it.
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt); // Restores the terminal back to its original settings
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

// Use either the constant or its numeric value (0x4)
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

typedef struct
{
    int x, y;
    int hp, max_hp;
    int strength;
    int level;
    int xp;
    int xp_to_level;
    int score;
    int highest_y;
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
    player.y = MAP_HEIGHT / 2;
    player.highest_y = player.y;
    player.max_hp = 20;
    player.hp = player.max_hp;
    player.strength = 5;
    player.level = 1;
    player.xp = 0;
    player.xp_to_level = 10;
    player.score = 0;
    strcpy(player.name, "Player");
}

void generate_new_row(int y)
{
    for (int x = 0; x < MAP_WIDTH; x++)
    {
        if (x == 0 || x == MAP_WIDTH - 1)
        {
            game_map[y][x] = '|';
        }
        else
        {
            int r = rand() % 100;
            if (r < 5)
            {
                // Place wall pair if there's room
                if (x < MAP_WIDTH - 2)
                {
                    game_map[y][x] = '[';
                    game_map[y][x + 1] = ']';
                    x++; // Skip next position
                }
                else
                {
                    game_map[y][x] = '_'; // Not enough room for pair
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

void init_map()
{
    for (int y = 0; y < MAP_HEIGHT; y++)
    {
        generate_new_row(y);
    }
}

void shift_world_down()
{
    for (int y = MAP_HEIGHT - 1; y > 0; y--)
    {
        memcpy(game_map[y], game_map[y - 1], MAP_WIDTH);
    }
    generate_new_row(0);
    world_offset++;

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

void spawn_enemies()
{
    if (enemy_count >= MAX_ENEMIES)
        return;

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

        enemies[enemy_count] = (Enemy){x, y, 5 + rand() % 5, 2 + rand() % 3, 5 + rand() % 5, false};
        enemy_count++;
    }

    if (player.score / 1000 > 0 && enemy_count < MAX_ENEMIES)
    {
        int x, y;
        do
        {
            x = 1 + rand() % (MAP_WIDTH - 2);
            y = 1 + rand() % (MAP_HEIGHT - 2);
        } while (game_map[y][x] != '_' ||
                 abs(x - player.x) < 5 ||
                 abs(y - player.y) < 5);

        enemies[enemy_count] = (Enemy){x, y, 20 + rand() % 10, 8 + rand() % 5, 50 + rand() % 20, true};
        enemy_count++;
    }
}

void update_score()
{
    if (player.y < player.highest_y)
    {
        player.score += (player.highest_y - player.y);
        player.highest_y = player.y;
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
            enemies[i].hp -= player.strength;
            if (enemies[i].hp <= 0)
            {
                player.xp += enemies[i].xp_value;
                move_cursor(0, 0);
                printf("Defeated %s! +%d XP", enemies[i].is_boss ? "BOSS" : "enemy", enemies[i].xp_value);

                for (int j = i; j < enemy_count - 1; j++)
                {
                    enemies[j] = enemies[j + 1];
                }
                enemy_count--;
                i--;

                if (player.xp >= player.xp_to_level)
                {
                    player.level++;
                    player.xp -= player.xp_to_level;
                    player.xp_to_level = (int)(player.xp_to_level * 1.5);
                    player.max_hp += 5;
                    player.hp = player.max_hp;
                    player.strength += 2;
                    move_cursor(0, 1);
                    printf("LEVEL UP! Now level %d", player.level);
                }
            }
            else
            {
                player.hp -= enemies[i].strength;
                move_cursor(0, 0);
                printf("You hit enemy! Enemy hits back for %d damage", enemies[i].strength);

                if (player.hp <= 0)
                {
                    move_cursor(0, 1);
                    printf("GAME OVER! Final Score: %d", player.score);
                    fflush(stdout);
                    msleep(3000);
                    exit(0);
                }
            }
            break;
        }
    }
}

void draw_game()
{
    clear_screen();

    for (int y = 0; y < MAP_HEIGHT; y++)
    {
        for (int x = 0; x < MAP_WIDTH; x++)
        {
            move_cursor(x, y);
            putchar(game_map[y][x]);
        }
    }

    move_cursor(player.x, player.y);
    putchar('@');

    for (int i = 0; i < enemy_count; i++)
    {
        move_cursor(enemies[i].x, enemies[i].y);
        putchar(enemies[i].is_boss ? 'B' : 'e');
    }

    move_cursor(0, MAP_HEIGHT);
    printf("HP: %d/%d | STR: %d | LVL: %d | XP: %d/%d | Score: %d | World: %d",
           player.hp, player.max_hp, player.strength, player.level,
           player.xp, player.xp_to_level, player.score, world_offset);
    move_cursor(0, MAP_HEIGHT + 1);
    printf("Controls: WASD to move, Q to quit, L for leaderboard");

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

void save_leaderboard()
{
    FILE *file = fopen(get_leaderboard_path(), "w");
    if (file == NULL)
        return;

    for (int i = 0; i < leaderboard_size; i++)
    {
        fprintf(file, "%s %d %d\n", leaderboard[i].name, leaderboard[i].level, leaderboard[i].distance);
    }

    fclose(file);
}

void add_to_leaderboard()
{
    for (int i = 0; i < leaderboard_size; i++)
    {
        if (strcmp(leaderboard[i].name, player.name) == 0)
        {
            if (player.score > leaderboard[i].distance)
            {
                leaderboard[i].level = player.level;
                leaderboard[i].distance = player.score;
            }
            save_leaderboard();
            return;
        }
    }

    if (leaderboard_size < MAX_LEADERBOARD)
    {
        strcpy(leaderboard[leaderboard_size].name, player.name);
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
            strcpy(leaderboard[lowest_index].name, player.name);
            leaderboard[lowest_index].level = player.level;
            leaderboard[lowest_index].distance = player.score;
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

    save_leaderboard();
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
    printf("Press any key to continue...");
    fflush(stdout);
    getch();
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
    fgets(input, 50, stdin);
    input[strcspn(input, "\n")] = '\0';
    strncpy(player.name, input, 49);
    player.name[49] = '\0';

#ifndef _WIN32
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
#endif
}

void handle_movement(int dx, int dy)
{
    int new_x = player.x + dx;
    int new_y = player.y + dy;

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

    get_player_name();
    load_leaderboard();

    init_player();
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
            running = false;
            break;
        case 'l':
            show_leaderboard();
            break;
        }
    }

    add_to_leaderboard();
    return 0;
}