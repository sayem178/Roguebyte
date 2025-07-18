#ifndef ENEMY_H
#define ENEMY_H
#define MAX_ENEMIES 3
typedef struct 
{
    int x,y;
    int hp;
    int alive;
}Enemy;

void initEnemy(Enemy enemies[], int count);



#endif // !ENEMY_H