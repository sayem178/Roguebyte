#include <stdio.h>
#include <sys/ioctl.h> //for low-level input/output control
#include <unistd.h>
#include "screen.h"

int getTerminalSize(int *rows, int *cols){
    struct winsize w;
    if ( ioctl(STDOUT_FILENO,TIOCGWINSZ, &w)==-1 )
    {
        return -1;

    }
    *rows = w.ws_row;
    *cols = w.ws_col;

    return 0;

}
