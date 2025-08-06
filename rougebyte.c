#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>

#define MAP_WIDTH 80
#define MAP_HEIGHT 24
#define MAX_ENEMIES 50
#define MAX_LEADERBOARD 10

// Game state
typedef struct {
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

typedef struct {
    int x, y;
    int hp;
    int strength;
    int xp_value;
    bool is_boss;
} Enemy;

typedef struct {
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

// Terminal control
void enable_raw_mode() {
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
}

void disable_raw_mode() {
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag |= ICANON | ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
}

int getch() {
    char buf = 0;
    if (read(STDIN_FILENO, &buf, 1) < 0) return 0;
    return buf;
}

void clear_screen() {
    printf("\033[2J\033[H");
}

void move_cursor(int x, int y) {
    printf("\033[%d;%dH", y + 1, x + 1);
}

// Game initialization
void init_player() {
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

void generate_map() {
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            if (x == 0 || x == MAP_WIDTH - 1 || y == MAP_HEIGHT - 1) {
                game_map[y][x] = '#'; // Only left, right and bottom borders
            } else if (y == 0) {
                game_map[y][x] = '.'; // Top row is always open
            } else {
                int r = rand() % 100;
                game_map[y][x] = (r < 5) ? '#' : (r < 10) ? '~' : '.';
            }
        }
    }
}

void expand_world_upward() {
    // Shift all rows down (except top row which we'll overwrite)
    for (int y = MAP_HEIGHT - 1; y > 1; y--) {
        memcpy(game_map[y], game_map[y-1], MAP_WIDTH);
    }
    
    // Generate new second row (just below the always-open top row)
    for (int x = 1; x < MAP_WIDTH - 1; x++) {
        int r = rand() % 100;
        game_map[1][x] = (r < 5) ? '#' : (r < 10) ? '~' : '.';
    }
    
    // Keep top row always open
    for (int x = 1; x < MAP_WIDTH - 1; x++) {
        game_map[0][x] = '.';
    }
    
    // Adjust positions (except player who stays at top)
    for (int i = 0; i < enemy_count; i++) {
        enemies[i].y++;
    }
}

void spawn_enemy(int x, int y, bool is_boss) {
    if (enemy_count >= MAX_ENEMIES) return;
    
    enemies[enemy_count].x = x;
    enemies[enemy_count].y = y;
    enemies[enemy_count].hp = is_boss ? (20 + rand() % 10) : (5 + rand() % 5);
    enemies[enemy_count].strength = is_boss ? (8 + rand() % 5) : (2 + rand() % 3);
    enemies[enemy_count].xp_value = is_boss ? (50 + rand() % 20) : (5 + rand() % 5);
    enemies[enemy_count].is_boss = is_boss;
    enemy_count++;
}

void spawn_enemies() {
    // Regular enemies
    int enemies_to_spawn = 3 + rand() % 3;
    for (int i = 0; i < enemies_to_spawn && enemy_count < MAX_ENEMIES; i++) {
        int x, y;
        do {
            x = 1 + rand() % (MAP_WIDTH - 2);
            y = player.highest_y - 5 - rand() % 10;
        } while (y < 0 || y >= MAP_HEIGHT || game_map[y][x] != '.');
        spawn_enemy(x, y, false);
    }
    
    // Boss every 1000 points
    if (player.score > 0 && player.score % 1000 == 0 && enemy_count < MAX_ENEMIES) {
        int x = 1 + rand() % (MAP_WIDTH - 2);
        int y = player.highest_y - 3;
        if (y >= 0 && y < MAP_HEIGHT && game_map[y][x] == '.') {
            spawn_enemy(x, y, true);
        }
    }
}

void move_enemies() {
    for (int i = 0; i < enemy_count; i++) {
        int dx = player.x - enemies[i].x;
        int dy = player.y - enemies[i].y;
        
        if (abs(dx) <= 5 && abs(dy) <= 5) {
            if (abs(dx) > abs(dy)) {
                if (dx > 0 && game_map[enemies[i].y][enemies[i].x + 1] == '.') enemies[i].x++;
                else if (dx < 0 && game_map[enemies[i].y][enemies[i].x - 1] == '.') enemies[i].x--;
            } else {
                if (dy > 0 && game_map[enemies[i].y + 1][enemies[i].x] == '.') enemies[i].y++;
                else if (dy < 0 && game_map[enemies[i].y - 1][enemies[i].x] == '.') enemies[i].y--;
            }
        } else {
            int dir = rand() % 4;
            switch (dir) {
                case 0: if (enemies[i].y > 0 && game_map[enemies[i].y - 1][enemies[i].x] == '.') enemies[i].y--; break;
                case 1: if (enemies[i].y < MAP_HEIGHT-1 && game_map[enemies[i].y + 1][enemies[i].x] == '.') enemies[i].y++; break;
                case 2: if (enemies[i].x > 0 && game_map[enemies[i].y][enemies[i].x - 1] == '.') enemies[i].x--; break;
                case 3: if (enemies[i].x < MAP_WIDTH-1 && game_map[enemies[i].y][enemies[i].x + 1] == '.') enemies[i].x++; break;
            }
        }
    }
}

void check_collisions() {
    for (int i = 0; i < enemy_count; i++) {
        if (player.x == enemies[i].x && player.y == enemies[i].y) {
            enemies[i].hp -= player.strength;
            if (enemies[i].hp <= 0) {
                player.xp += enemies[i].xp_value;
                move_cursor(0, 0);
                printf("Defeated %s! +%d XP", enemies[i].is_boss ? "BOSS" : "enemy", enemies[i].xp_value);
                
                for (int j = i; j < enemy_count - 1; j++) {
                    enemies[j] = enemies[j + 1];
                }
                enemy_count--;
                i--;
                
                if (player.xp >= player.xp_to_level) {
                    player.level++;
                    player.xp -= player.xp_to_level;
                    player.xp_to_level = (int)(player.xp_to_level * 1.5);
                    player.max_hp += 5;
                    player.hp = player.max_hp;
                    player.strength += 2;
                    move_cursor(0, 1);
                    printf("LEVEL UP! Now level %d", player.level);
                }
            } else {
                player.hp -= enemies[i].strength;
                move_cursor(0, 0);
                printf("You hit enemy! Enemy hits back for %d damage", enemies[i].strength);
                
                if (player.hp <= 0) {
                    move_cursor(0, 1);
                    printf("GAME OVER! Final Score: %d", player.score);
                    fflush(stdout);
                    sleep(3);
                    disable_raw_mode();
                    exit(0);
                }
            }
            break;
        }
    }
}

void process_movement(int dx, int dy) {
    int new_x = player.x + dx;
    int new_y = player.y + dy;
    
    if (new_x < 1 || new_x >= MAP_WIDTH - 1) return;
    if (new_y < 0 || new_y >= MAP_HEIGHT - 1) return; // Allow moving to top row (y=0)
    if (game_map[new_y][new_x] != '.') return;
    
    player.x = new_x;
    player.y = new_y;
    
    // Scoring and world expansion
    if (new_y < player.highest_y) {
        player.score += (player.highest_y - new_y);
        player.highest_y = new_y;
        
        if (player.highest_y < 5) {
            expand_world_upward();
            player.y++; // Keep player at same visual position
        }
    }
    
    static int move_count = 0;
    if (++move_count % 40 == 0) {
        spawn_enemies();
    }
    
    move_enemies();
    check_collisions();
}

void draw_game() {
    clear_screen();
    
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            move_cursor(x, y);
            putchar(game_map[y][x]);
        }
    }
    
    move_cursor(player.x, player.y);
    putchar('@');
    
    for (int i = 0; i < enemy_count; i++) {
        move_cursor(enemies[i].x, enemies[i].y);
        putchar(enemies[i].is_boss ? 'B' : 'e');
    }
    
    move_cursor(0, MAP_HEIGHT);
    printf("HP: %d/%d | STR: %d | LVL: %d | XP: %d/%d | Score: %d", 
           player.hp, player.max_hp, player.strength, player.level, 
           player.xp, player.xp_to_level, player.score);
    move_cursor(0, MAP_HEIGHT + 1);
    printf("Controls: WASD to move, Q to quit, L for leaderboard");
    
    fflush(stdout);
}

void load_leaderboard() {
    FILE *file = fopen("leaderboard.txt", "r");
    if (file == NULL) return;
    
    leaderboard_size = 0;
    while (leaderboard_size < MAX_LEADERBOARD && 
           fscanf(file, "%49s %d %d", 
                 leaderboard[leaderboard_size].name, 
                 &leaderboard[leaderboard_size].level, 
                 &leaderboard[leaderboard_size].distance) == 3) {
        leaderboard_size++;
    }
    
    fclose(file);
}

void save_leaderboard() {
    FILE *file = fopen("leaderboard.txt", "w");
    if (file == NULL) return;
    
    for (int i = 0; i < leaderboard_size; i++) {
        fprintf(file, "%s %d %d\n", leaderboard[i].name, leaderboard[i].level, leaderboard[i].distance);
    }
    
    fclose(file);
}

void add_to_leaderboard() {
    // Find if player already exists in leaderboard
    for (int i = 0; i < leaderboard_size; i++) {
        if (strcmp(leaderboard[i].name, player.name) == 0) {
            if (player.score > leaderboard[i].distance) {
                leaderboard[i].level = player.level;
                leaderboard[i].distance = player.score;
            }
            return;
        }
    }
    
    // Add new entry
    if (leaderboard_size < MAX_LEADERBOARD) {
        strncpy(leaderboard[leaderboard_size].name, player.name, 49);
        leaderboard[leaderboard_size].level = player.level;
        leaderboard[leaderboard_size].distance = player.score;
        leaderboard_size++;
    } else {
        int lowest_index = 0;
        for (int i = 1; i < leaderboard_size; i++) {
            if (leaderboard[i].distance < leaderboard[lowest_index].distance) {
                lowest_index = i;
            }
        }
        
        if (player.score > leaderboard[lowest_index].distance) {
            strncpy(leaderboard[lowest_index].name, player.name, 49);
            leaderboard[lowest_index].level = player.level;
            leaderboard[lowest_index].distance = player.score;
        }
    }
    
    // Sort leaderboard
    for (int i = 0; i < leaderboard_size - 1; i++) {
        for (int j = 0; j < leaderboard_size - i - 1; j++) {
            if (leaderboard[j].distance < leaderboard[j + 1].distance) {
                LeaderboardEntry temp = leaderboard[j];
                leaderboard[j] = leaderboard[j + 1];
                leaderboard[j + 1] = temp;
            }
        }
    }
    
    save_leaderboard();
}

void show_leaderboard() {
    clear_screen();
    move_cursor(0, 0);
    printf("=== LEADERBOARD ===");
    move_cursor(0, 1);
    printf("Rank  Name           Level  Distance");
    
    for (int i = 0; i < leaderboard_size; i++) {
        move_cursor(0, 2 + i);
        printf("%2d.   %-12s   %3d     %5d", 
               i + 1, leaderboard[i].name, leaderboard[i].level, leaderboard[i].distance);
    }
    
    move_cursor(0, MAP_HEIGHT);
    printf("Press any key to continue...");
    fflush(stdout);
    getch();
}

void get_player_name() {
    disable_raw_mode();
    clear_screen();
    move_cursor(0, 0);
    printf("Enter your name (max 49 chars, press Enter when done): ");
    fflush(stdout);
    
    char input[50];
    if (fgets(input, 50, stdin) != NULL) {
        input[strcspn(input, "\n")] = '\0'; // Remove newline
        strncpy(player.name, input, 49);
        player.name[49] = '\0';
    }
    
    enable_raw_mode();
}

int main() {
    srand(time(NULL));
    
    enable_raw_mode();
    get_player_name();
    load_leaderboard();
    
    init_player();
    generate_map();
    spawn_enemies();
    
    bool running = true;
    while (running) {
        draw_game();
        
        int ch = getch();
        int dx = 0, dy = 0;
        
        switch (ch) {
            case 'w': case 'W': dy = -1; break;
            case 'a': case 'A': dx = -1; break;
            case 's': case 'S': dy = 1; break;
            case 'd': case 'D': dx = 1; break;
            case 'q': case 'Q': running = false; break;
            case 'l': case 'L': show_leaderboard(); continue;
            case 0: continue; // Skip invalid input
        }
        
        if (dx != 0 || dy != 0) {
            process_movement(dx, dy);
        }
    }
    
    add_to_leaderboard();
    disable_raw_mode();
    return 0;
}