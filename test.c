#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main() {
    srand(time(NULL)); // seed with current time (changes every run)

    for (int i = 0; i < 5; i++) {
        int r = rand() % 100;  // random number between 0-99
        printf("Random[%d] = %d\n", i, r);
    }
    return 0;
}
