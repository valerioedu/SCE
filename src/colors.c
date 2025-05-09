#include "colors.h"

const char* oop_blue_keywords[] = { 
    "private", "protected", "public"
};

const char* blue_keywords[] = { 
    "void", "char", "short", "int", "long", "float", "double",
    
    "const", "volatile",
    
    "auto", "register", "static", "extern",

    "signed", "unsigned", "bool", "struct", "union", "enum", "class",
    "inline", "virtual", "explicit", "friend", "typedef", "namespace",
    "template", "typename", "mutable", "using", "asm", "alignas", "alignof",
    "decltype", "constexpr", "noexcept", "thread_local", "nullptr"
};

const char* purple_keywords[] = { 
    "if", "else", "switch", "case", "default", "for", "while", "do", "goto", "continue", "break", "return"
};

KeywordInfo check_keyword(char* line, const char* keyword) {
    KeywordInfo info = {0};
    int len = strlen(keyword);
    int line_len = strlen(line);
    int i = 0;
    while (i < line_len) {
        if (strncmp(&line[i], keyword, len) == 0 &&
            (i == 0 || !isalnum(line[i-1])) &&
            (i + len == line_len || !isalnum(line[i+len]))) {
            if (info.count < MAX_KEYWORDS) {
                info.keywords[info.count].start = i;
                info.keywords[info.count].end = i + len;
                info.count++;
            }
            i += len;
        } else {
            i++;
        }
    }
    return info;
}

KeywordInfo check_blue_keywords(char* line) {
    KeywordInfo total_info = {0};
    int num_keywords = sizeof(blue_keywords) / sizeof(blue_keywords[0]);
    char* word_start = line;
    char* word_end;
    while (*word_start) {
        while (*word_start && !isalnum(*word_start)) {
            word_start++;
        }
        if (!*word_start) break;
        word_end = word_start;
        while (*word_end && isalnum(*word_end)) {
            word_end++;
        }
        for (int i = 0; i < num_keywords; i++) {
            if (strncmp(word_start, blue_keywords[i], word_end - word_start) == 0 &&
                strlen(blue_keywords[i]) == (size_t)(word_end - word_start)) {
                if (total_info.count < MAX_KEYWORDS) {
                    total_info.keywords[total_info.count].start = word_start - line;
                    total_info.keywords[total_info.count].end = word_end - line;
                    total_info.count++;
                }
                break;
            }
        }
        word_start = word_end;
    }
    return total_info;
}

KeywordInfo check_purple_keywords(char* line) {
    KeywordInfo total_info = {0};
    int num_keywords = sizeof(purple_keywords) / sizeof(purple_keywords[0]);
    char* word_start = line;
    char* word_end;
    while (*word_start) {
        while (*word_start && !isalnum(*word_start)) {
            word_start++;
        }
        if (!*word_start) break;
        word_end = word_start;
        while (*word_end && isalnum(*word_end)) {
            word_end++;
        }
        for (int i = 0; i < num_keywords; i++) {
            if (strncmp(word_start, purple_keywords[i], word_end - word_start) == 0 &&
                strlen(purple_keywords[i]) == (size_t)(word_end - word_start)) {
                if (total_info.count < MAX_KEYWORDS) {
                    total_info.keywords[total_info.count].start = word_start - line;
                    total_info.keywords[total_info.count].end = word_end - line;
                    total_info.count++;
                }
                break;
            }
        }
        word_start = word_end;
    }
    return total_info;
}

KeywordInfo check_functions(char* line) {
    KeywordInfo total_info = {0};
    char* word_start = line;
    char* word_end;
    while (*word_start) {
        while (*word_start && !isalnum(*word_start)) {
            word_start++;
        }
        if (!*word_start) break;
        word_end = word_start;
        while (*word_end && isalnum(*word_end)) {
            word_end++;
        }
        char* next_char = word_end;
        while (*next_char && isspace(*next_char)) {
            next_char++;
        }
        if (*next_char == '(') {
            if (total_info.count < MAX_KEYWORDS) {
                total_info.keywords[total_info.count].start = word_start - line;
                total_info.keywords[total_info.count].end = word_end - line;
                total_info.count++;
            }
        }
        word_start = word_end;
    }
    return total_info;
}

KeywordInfo color_parentheses(char* line) {
    KeywordInfo total_info = {0};
    char* current = line;
    while (*current) {
        if (*current == '(' || *current == ')' || *current == '{' || *current == '}' || *current == '[' || *current == ']') {
            if (total_info.count < MAX_KEYWORDS) {
                total_info.keywords[total_info.count].start = current - line;
                total_info.keywords[total_info.count].end = current - line + 1;
                total_info.count++;
            }
        }
        current++;
    }
    return total_info;
}

KeywordInfo check_variables(char* line) {
    KeywordInfo total_info = {0};
    char* word_start = line;
    char* word_end;
    while (*word_start && total_info.count < MAX_KEYWORDS) {
        while (*word_start && !isalnum(*word_start) && *word_start != '_') {
            word_start++;
        }
        if (!*word_start) break;
        word_end = word_start;
        while (*word_end && (isalnum(*word_end) || *word_end == '_')) {
            word_end++;
        }
        for (int i = 0; i < MAX_VARIABLES && variables[i] != NULL; i++) {
            if (strncmp(word_start, variables[i], word_end - word_start) == 0 &&
                strlen(variables[i]) == (size_t)(word_end - word_start)) {
                total_info.keywords[total_info.count].start = word_start - line;
                total_info.keywords[total_info.count].end = word_end - line;
                total_info.count++;
                break;
            }
        }
        word_start = word_end;
    }
    return total_info;
}

KeywordInfo check_syntax(char* line) {
    KeywordInfo total_info = {0};
    KeywordInfo blue_info = check_blue_keywords(line);
    KeywordInfo purple_info = check_purple_keywords(line);
    KeywordInfo function_info = check_functions(line);
    KeywordInfo parentheses_info = color_parentheses(line);
    KeywordInfo variable_info = check_variables(line);
    KeywordInfo all_infos[] = {blue_info, purple_info, function_info, parentheses_info, variable_info};
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < all_infos[i].count && total_info.count < MAX_KEYWORDS; j++) {
            total_info.keywords[total_info.count++] = all_infos[i].keywords[j];
        }
    }
    return total_info;
}