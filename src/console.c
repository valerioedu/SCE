#include <ncurses.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "console.h"
#include "library.h"
#include "macros.h"

void terminal() {
    def_prog_mode();
    endwin();

    system("bash");

    reset_prog_mode();
    refresh();
}

void console() {
    int x,y;
    getmaxyx(stdscr, y, x);
    move(y-1, 0);
    clrtoeol();
    printw("Console: ");
    keypad(stdscr, true);
    char buffer[24] = {0};
    char c = 0;
    uint8_t i = 0;
    do {
        c = getch();
    
        if ((c == 7) && i > 0) {
            i--;
            buffer[i] = '\0';
        }
    
        if (c >= 32 && c < 127) {
            buffer[i] = c;
            i++;
            buffer[i] = '\0';
        }
        move(y-1, 0);
        clrtoeol();
        mvprintw(y-1, 0, "Console: %s", buffer);
        move(y-1, 9 + i);
        refresh();
        if (i == 24) {
            clear();
            mvprintw(y-1, 0, "Only 24 characters allowed");
            getch();
            move(y-1,0);
            clrtoeol();
            return;
        }
    } while (c != '\n' && c != ESCAPE && i < 24);

    if (c == ESCAPE) return;

    if (strcmp(buffer, "term") == 0) terminal();
    if (strcmp(buffer, "search") == 0) ctrl_f();
}