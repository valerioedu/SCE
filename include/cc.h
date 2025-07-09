#ifndef CC_H
#define CC_H

#ifdef _WIN32
    #include <curses.h>
#else
    #include <ncurses.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "library.h"
#include "macros.h"
#include "pages.h"

bool check_parentheses(char text);
bool check_brackets(char text);
bool check_braces(char text);
bool checks(char text);

#endif