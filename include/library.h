#ifndef LIBRARY_H
#define LIBRARY_H

#include <ncurses.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_LINES 32*1024
#define MAX_COLS 128
#define ESCAPE 27

#define TABS_SIZE 4 //temporary

typedef struct editor_config {
    int tab_size;
    bool parenthesis_autocomplete;
    bool quotations_autocomplete;
    int keywords_color;
    int var_color;
} editor_config;

extern char lines[MAX_LINES][MAX_COLS];
extern int line_count;
extern int current_line;
extern int current_col;
extern int start_line;
extern char copy[128*128];
extern char file_name[64];
extern char text[MAX_LINES * MAX_COLS];

void insert_char(char c);

void tab();

void transcribe_to_text();

#endif