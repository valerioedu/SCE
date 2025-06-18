#ifndef CFILES_H
#define CFILES_H

#include <ncurses.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_COLS 256
#define C_PRIMITIVE_TYPES_SIZE 24

extern const char* c_primitive_types[];
extern const char* c_blue_keywords[];
extern const char* c_purple_keywords[];
extern const char* c_yellow_keywords[];
extern const char* c_light_blue_keywords[];

#endif