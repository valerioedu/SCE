#ifndef CFILES_H
#define CFILES_H

#include <ncurses.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_DEFINITIONS 1024
#define MAX_COLS 256
#define C_PRIMITIVE_TYPES_SIZE 16
#define C_BLUE_KEYWORDS_SIZE 12
#define C_PURPLE_KEYWORDS_SIZE 12
#define C_YELLOW_KEYWORDS_SIZE 5
#define C_LIGHT_BLUE_KEYWORDS_SIZE 6

extern const char* c_primitive_types[];
extern const char* c_blue_keywords[];
extern const char* c_purple_keywords[];
extern const char* c_yellow_keywords[];
extern const char* c_light_blue_keywords[];
extern char* definitions[MAX_DEFINITIONS];

void save_definition(char* def);
void detect_defines(char* line);

#endif