#include <ncurses.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "console.h"

void console() {
    int x,y;
    getmaxyx(stdscr, y, x);
    move(y-1, 0);
    clrtoeol();
    printw("Console: ");
    getch();
}