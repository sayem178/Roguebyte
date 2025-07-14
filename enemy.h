#ifndef ENEMY_H
#define ENEMY_H

typedef struct 
{
    int x,y;
    int hp;
    int alive;
}Enemy;

void initEnemy(Enemy*e);



#endif // !ENEMY_H