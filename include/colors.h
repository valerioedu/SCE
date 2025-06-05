#ifndef COLORS_H
#define COLORS_H

#include <ncurses.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "library.h"
#include "variables.h"


#define MAX_KEYWORDS 1000

extern const char* oop_blue_keywords[];

extern const char* blue_keywords[];

extern const char* purple_keywords[];

extern const int blue_keywords_count;
extern const int purple_keywords_count;
extern int inside_multiline_comment;

typedef struct {
    int start;
    int end;
} Keyword;

typedef struct {
    Keyword keywords[MAX_KEYWORDS];
    int count;
} KeywordInfo;

KeywordInfo check_keyword(char* line, const char* keyword);
KeywordInfo check_blue_keywords(char* line);
KeywordInfo check_purple_keywords(char* line);
KeywordInfo check_functions(char* line);
KeywordInfo color_parentheses(char* line);
KeywordInfo check_variables(char* line);
KeywordInfo check_syntax(char* line);
KeywordInfo color_quotes(char* line);
KeywordInfo color_comments(char* line);

#endif