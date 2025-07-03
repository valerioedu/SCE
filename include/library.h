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

#define TABS_SIZE config.tab_size

typedef struct EditorConfig {
    int tab_size;
    int color_0;
    int color_1;
    int color_2;
    int color_3;
    int color_4;
    int color_5;
    int color_6;
    int color_7;
    int color_8;
    bool parenthesis_autocomplete;
    bool quotations_autocomplete;
    bool expandtab;
    char* default_path;
    bool autosave;
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
extern char file_name[512];
extern char* text;
extern size_t text_capacity;
extern EditorConfig config;
extern bool open_file_browser;
extern char* selected_path_from_browser;
extern bool in_memory;
extern bool editing_multiple_cursors;

void insert_char(char c);
void transcribe_to_text();
void update_screen_content(int start_line);
void cleanup_lines();
void ensure_lines_capacity(size_t needed_lines);
void ensure_text_capacity(size_t needed_size);
void cleanup_text();
void init_lines();
void insert_char_at_position(char c, int line, int col);

#endif