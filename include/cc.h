#ifndef CC_H
#define CC_H

#include <ncurses.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "library.h"
#include "macros.h"
#include "pages.h"

bool check_parentheses(char text);
bool check_brackets(char text);
bool check_braces(char text);
bool check_comments(char* text);
bool checks(char text);

#endif