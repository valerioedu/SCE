#ifndef LIBRARY_H
#define LIBRARY_H

#include <ncurses.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "macros.h"

#define MAX_LINES 32*1024
#define MAX_COLS 256
#define ESCAPE 27

#define MAX_UNDO 64

#define TABS_SIZE 4 //temporary

typedef struct EditorConfig {
    int tab_size;
    int keywords_color;
    int var_color;
    bool parenthesis_autocomplete;
    bool quotations_autocomplete;
    char* default_path;
} EditorConfig;

typedef struct UndoState {
    int line_count;
    int cursor_line;
    int cursor_col;
    char** lines;
} UndoState;

extern UndoState undo_history[MAX_UNDO];
extern int undo_count;
extern int undo_position;
extern char** lines;
extern int line_count;
extern int current_line;
extern int current_col;
extern int start_line;
extern char copy[128*128];
extern char file_name[64];
extern char text[MAX_LINES * MAX_COLS];
extern EditorConfig config;
extern bool open_file_browser;
extern char* selected_path_from_browser;

void insert_char(char c);
void tab();
void transcribe_to_text();
void update_screen_content(int start_line);
void cleanup_lines();
void ensure_lines_capacity(size_t needed_lines);

#endif