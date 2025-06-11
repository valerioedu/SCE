#include <ncurses.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "console.h"
#include "library.h"
#include "macros.h"
#include "sceconfig.h"

bool exit_command = false;

void terminal() {
    def_prog_mode();
    endwin();

    system("bash");

    reset_prog_mode();
    refresh();
}

void console() {
    init_pair(10, COLOR_WHITE, COLOR_BLUE);
    int x, y;
    getmaxyx(stdscr, y, x);
    
    attron(COLOR_PAIR(10));
    for (int i = 0; i < x; i++) {
        mvaddch(y-1, i, ' ');
    }
    
    mvprintw(y-1, 0, "Console: ");
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
        for (int j = 0; j < x; j++) {
            mvaddch(y-1, j, ' ');
        }
        
        mvprintw(y-1, 0, "Console: %s", buffer);
        move(y-1, 9 + i);
        refresh();
        
        if (i == 24) {
            clear();
            mvprintw(y-1, 0, "Only 24 characters allowed");
            getch();

            for (int j = 0; j < x; j++) {
                mvaddch(y-1, j, ' ');
            }
            mvprintw(y-1, 0, "Console: ");
            attroff(COLOR_PAIR(10));
            return;
        }
    } while (c != '\n' && c != ESCAPE && i < 24);

    attroff(COLOR_PAIR(10));

    if (c == ESCAPE) return;

    char* tokens[3] = {NULL};
    char buffer_copy[24];
    strcpy(buffer_copy, buffer);
    
    char* token = strtok(buffer_copy, " ");
    int token_count = 0;
    
    while (token != NULL && token_count < 3) {
        tokens[token_count++] = token;
        token = strtok(NULL, " ");
    }
    
    if (token_count > 0) {
        if (strcmp(tokens[0], "term") == 0) terminal();
        else if (strcmp(tokens[0], "search") == 0) ctrl_f();
        else if (strcmp(tokens[0], "config") == 0) config_editor();
        else if (strcmp(tokens[0], "exit") == 0 || strcmp(tokens[0], "quit") == 0) exit_command = true;
        else if (strcmp(tokens[0], "goto") == 0) {
            if (token_count == 2 && (strcmp(tokens[1], "help")) || (strcmp(tokens[1], "-h"))) {
                int rows,cols;
                getmaxyx(stdscr, rows,cols);
                move(rows -1, 0);
                clrtoeol();
                mvprintw(rows -1, 3, "goto use: goto <Lines> <Cols>");
                getch();
            }

            if (token_count == 3) {
                int col_num, line_num;
                
                if (sscanf(tokens[1], "%d", &line_num) == 1 && 
                    sscanf(tokens[2], "%d", &col_num) == 1) {
                    
                    line_num--;
                    col_num--;
                    
                    if (line_num >= 0 && line_num < line_count) {
                        current_line = line_num;
                        
                        if (col_num < 0) col_num = 0;
                        if (col_num > strlen(lines[current_line])) 
                            col_num = strlen(lines[current_line]);
                        
                        current_col = col_num;
                        
                        int start_line = (current_line > LINES/2) ? 
                                current_line - LINES/2 : 0;
                        update_screen_content(start_line);
                    }
                }
            }
        }
    }
}