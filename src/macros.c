#include <ncurses.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "macros.h"
#include "library.h"

int* search(char str[], int* matches) {
    size_t size = strlen(str);
    int capacity = 4;
    int* result = malloc(4 * sizeof(int));
    int index = 0;

    if (!result || size == 0) return NULL;

    for (int i = 0; i < line_count; i++) {
        int a = strlen(lines[i]);
        if (a < size) continue;

        for (int j = 0; j < a - size + 1; j++) {
            bool match = true;
            for (int k = 0; k < size; k++) {
                if (str[k] != lines[i][j+k]) {
                    match = false;
                    break;
                }
            }

            if (match) {
                if (index >= capacity) {
                    capacity *= 2;
                    int* new_result = realloc(result, capacity * sizeof(int));
                    if (!new_result) {
                        free(result);
                        return NULL;
                    }
                    result = new_result;
                }
                
                result[index++] = i;
                break;
            }
        }
    }

    if (!index) {free(result); return NULL;}
    *matches = index;
    return result;
}

void replace(char* src, char* dest) {
    if (strcmp(src, dest) == 0) return;
    size_t size = strlen(dest);
    size_t size2 = strlen(src);
    if (size == size2) {
        memcpy(src, dest, size);
    } else {
        strcpy(src, dest);
    }
}

void ctrl_f() {
    keypad(stdscr, true);
    char buffer[24] = {0};
    int row, col;
    getmaxyx(stdscr, row, col);
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
        move(0, col - 30);
        clrtoeol();
        mvprintw(0, col - 30, "Find: %s", buffer);
        move(0, col - 24 + i);
        refresh();
    } while (c != '\n' && c != ESCAPE);

    if (c == ESCAPE) return;

    if (c == '\n' && i > 0) {
        int matches_count;
        int *matches = search(buffer, &matches_count);

        if (matches && matches_count > 0) {
            int index = 0;
            current_line = matches[index];
            mvprintw(1, col - 30, "Found %d matches", matches_count);
            mvprintw(2, col - 30, "Press X to replace match");
            mvprintw(3, col - 30, "Press C to replace all");
            refresh();

            int ch;
            while (1) {
                ch = getch();
            
                if (ch == KEY_UP) {
                    index = (index == 0) ? matches_count - 1 : index - 1;
                } else if (ch == KEY_DOWN) {
                    index = (index + 1) % matches_count;
                } else if (ch == 120 || ch == 88) {
                    char cc = 0;
                    uint8_t k = 0;
                    do {
                        cc = getch();
                        char replace[24];
                
                        if ((cc == 7) && k > 0) {
                            k--;
                            replace[i] = '\0';
                        }
                
                        if (cc >= 32 && cc < 127) {
                            replace[i] = cc;
                            k++;
                            replace[i] = '\0';
                        }
                        move(0, col - 30);
                        clrtoeol();
                        mvprintw(0, col - 30, "Replace: %s", replace);
                        move(0, col - 24 + i);
                        refresh();
                    } while (cc != '\n' && cc != ESCAPE);
                } else {
                    ungetch(ch);
                    break;
                }
            
                current_line = matches[index];
            
                int start_line = (current_line > LINES / 2)
                                   ? current_line - LINES / 2 : 0;
                update_screen_content(start_line);

                mvprintw(row - 1, 0, "Line: %d, Column: %d", current_line + 1, current_col + 1);
                mvprintw(row - 1, col - strlen(file_name) - 6, "File: %s", file_name);

                move(current_line - start_line, 6);
                refresh();
            }
            free(matches);
        } else {
            mvprintw(1, col - 30, "No matches found");
            refresh();
        }
    }
}

void save_undo_state() {
    if (undo_count >= MAX_UNDO) {
        if (undo_history[0].lines) {
            for (int i = 0; i < undo_history[0].line_count; i++) {
                free(undo_history[0].lines[i]);
            }
            free(undo_history[0].lines);
        }
        
        memmove(&undo_history[0], &undo_history[1], (MAX_UNDO - 1) * sizeof(UndoState));
        undo_count = MAX_UNDO - 1;
    }
    
    undo_history[undo_count].line_count = line_count;
    undo_history[undo_count].cursor_line = current_line;
    undo_history[undo_count].cursor_col = current_col;
    
    undo_history[undo_count].lines = malloc(line_count * sizeof(char*));
    if (!undo_history[undo_count].lines) return;
    
    for (int i = 0; i < line_count; i++) {
        undo_history[undo_count].lines[i] = strdup(lines[i]);
        if (!undo_history[undo_count].lines[i]) {
            for (int j = 0; j < i; j++) {
                free(undo_history[undo_count].lines[j]);
            }
            free(undo_history[undo_count].lines);
            undo_history[undo_count].lines = NULL;
            return;
        }
    }
    
    undo_count++;
    undo_position = undo_count;
}

void ctrl_z() {
    if (undo_position <= 0) return;

    undo_position--;

    line_count = undo_history[undo_position].line_count;
    current_line = undo_history[undo_position].cursor_line;
    current_col = undo_history[undo_position].cursor_col;

    for (int i = 0; i < line_count; i++) {
        strncpy(lines[i], undo_history[undo_position].lines[i], MAX_COLS - 1);
        lines[i][MAX_COLS - 1] = '\0';
    }

    int start_line = (current_line > LINES / 2) 
                     ? current_line - LINES / 2 : 0;
    update_screen_content(start_line);

    move(current_line - start_line, 6 + current_col);
    refresh();
}

void cleanup_undo_history() {
    for (int i = 0; i < undo_count; i++) {
        if (undo_history[i].lines) {
            for (int j = 0; j < undo_history[i].line_count; j++) {
                free(undo_history[i].lines[j]);
            }
            free(undo_history[i].lines);
            undo_history[i].lines = NULL;
        }
    }
}

void ctrl_right_arrow(char* line) {
    int pos = current_col;
    int len = strlen(line);
    
    if (pos >= len) return;
    
    if (line[pos] == ' ') {
        while (pos < len && line[pos] == ' ') {
            pos++;
        }
        
        current_col = pos;
        return;
    }
    
    if (line[pos] != ' ') {
        while (pos < len && line[pos] != ' ') {
            pos++;
        }
    }
    
    current_col = pos;
}

void ctrl_left_arrow(char* line) {
    int pos = current_col;
    
    if (pos <= 0) return;
    
    if (pos > 0 && line[pos-1] == ' ') {
        while (pos > 0 && line[pos-1] == ' ') {
            pos--;
        }

        current_col = pos;
        return;
    }
    
    if (pos > 0 && line[pos-1] != ' ') {
        while (pos > 0 && line[pos-1] != ' ') {
            pos--;
        }
    }
    
    current_col = pos;
}