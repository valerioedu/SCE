#ifndef LANGUAGES_H
#define LANGUAGES_H
#ifndef _WIN32
#include <regex.h>

#include "library.h"

typedef struct Pattern{
    char *regex;
    unsigned short color;
} Pattern;

typedef struct MatchInfo {
    int start;
    int end;
    int color;
} MatchInfo;

extern Pattern PyPatterns[];

int find_python_matches(const char* line, MatchInfo* matches, int max_matches);
int get_color_at_position(MatchInfo* matches, int match_count, int position);
int find_csharp_matches(const char* line, MatchInfo* matches, int max_matches);
int find_bash_matches(const char* line, MatchInfo* matches, int max_matches);
int find_powershell_matches(const char* line, MatchInfo* matches, int max_matches);
int find_go_matches(const char* line, MatchInfo* matches, int max_matches);
int find_rust_matches(const char* line, MatchInfo* matches, int max_matches);
int find_java_matches(const char* line, MatchInfo* matches, int max_matches);
int find_javascript_matches(const char* line, MatchInfo* matches, int max_matches);
#endif
#endif