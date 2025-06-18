#include "colors.h"

/*
*   +-----------------------------+
*   | TODO: Implement typedefs    |
*   |                             |
*   | Implement preprocessor defs |
*   +-----------------------------+
*/

const char* blue_keywords[] = { 
    "void", "char", "short", "int", "long", "float", "double",
    
    "const", "volatile",
    
    "auto", "register", "static", "extern",

    "signed", "unsigned", "bool", "struct", "union", "enum", "class",
    "inline", "virtual", "explicit", "friend", "typedef", "namespace",
    "template", "typename", "mutable", "using", "asm", "alignas", "alignof",
    "decltype", "constexpr", "noexcept", "thread_local", "nullptr",

    // other words that are usually blue
    "NULL", "true", "false", "uint8_t", "uint16_t", "uint32_t", "uint64_t",
    "int8_t", "int16_t", "int32_t", "int64_t", "size_t", "intptr_t", "uintptr_t" 
};

const char* purple_keywords[] = { 
    "if", "else", "switch", "case", "default", "for", "while", "do", "goto", "continue", "break", "return", "#define", "#include", "#ifndef", "ifdef", "#endif", "#if"
};

const char* c_primitive_types[] = {
    "void", "int", "char", "short", "long", "float", "double",
    "size_t", "int8_t", "int16_t", "int32_t", "int64_t",
    "uint8_t", "uint16_t", "uint32_t", "uint64_t",
    "bool", "int8_t", "int16_t", "int32_t", "int64_t", "size_t", "intptr_t", "uintptr_t"
};

const int blue_keywords_count = sizeof(blue_keywords) / sizeof(blue_keywords[0]);
const int purple_keywords_count = sizeof(purple_keywords) / sizeof(purple_keywords[0]);

int inside_multiline_comment = 0;
int inside_quote = 0;

static int is_inside_quotes(const char* line, int pos) {
    int in_single = 0, in_double = 0;
    for (int i = 0; i < pos; i++) {
        if (line[i] == '\'' && (i == 0 || line[i-1] != '\\')) in_single = !in_single;
        if (line[i] == '"' && (i == 0 || line[i-1] != '\\')) in_double = !in_double;
    }
    return in_single || in_double;
}

KeywordInfo color_comments(char* line) {
    KeywordInfo total_info = {0};
    int line_len = strlen(line);
    
    char* single_comment = strstr(line, "//");
    if (single_comment && !inside_multiline_comment) {
        int pos = single_comment - line;
        if (!is_inside_quotes(line, pos)) {
            if (total_info.count < MAX_KEYWORDS) {
                total_info.keywords[total_info.count].start = pos;
                total_info.keywords[total_info.count].end = line_len;
                total_info.count++;
            }
            return total_info;
        }
    }
    
    if (inside_multiline_comment) {
        char* end = strstr(line, "*/");
        if (end) {
            inside_multiline_comment = 0;
            if (total_info.count < MAX_KEYWORDS) {
                total_info.keywords[total_info.count].start = 0;
                total_info.keywords[total_info.count].end = (end - line) + 2;
                total_info.count++;
            }
        } else {
            if (total_info.count < MAX_KEYWORDS) {
                total_info.keywords[total_info.count].start = 0;
                total_info.keywords[total_info.count].end = line_len;
                total_info.count++;
            }
        }
    } else {
        char* start = strstr(line, "/*");
        if (start) {
            char* end = strstr(start + 2, "*/");
            if (end) {
                if (total_info.count < MAX_KEYWORDS) {
                    total_info.keywords[total_info.count].start = start - line;
                    total_info.keywords[total_info.count].end = (end - line) + 2;
                    total_info.count++;
                }
            } else {
                inside_multiline_comment = 1;
                if (total_info.count < MAX_KEYWORDS) {
                    total_info.keywords[total_info.count].start = start - line;
                    total_info.keywords[total_info.count].end = line_len;
                    total_info.count++;
                }
            }
        }
    }
    
    return total_info;
}

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
        while (*word_start && !isalnum(*word_start) && *word_start != '_') {
            word_start++;
        }
        if (!*word_start) break;
        word_end = word_start;
        while (*word_end && (isalnum(*word_end) || *word_end == '_')) {
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
        if (*word_start == '#') {
            word_end = word_start + 1;
            while (*word_end && isalnum(*word_end)) {
                word_end++;
            }
            for (int i = 0; i < num_keywords; i++) {
                if (strncmp(word_start, purple_keywords[i], word_end - word_start) == 0 &&
                    strlen(purple_keywords[i]) == (size_t)(word_end - word_start)) {
                    total_info.keywords[total_info.count].start = word_start - line;
                    total_info.keywords[total_info.count].end = word_end - line;
                    total_info.count++;
                    break;
                }
            }
            word_start = word_end;
        } else {
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
    }
    return total_info;
}

KeywordInfo check_functions(char* line) {
    KeywordInfo total_info = {0};
    char* word_start = line;
    char* word_end;
    while (*word_start) {
        while (*word_start && !isalnum(*word_start) && *word_start != '_') {
            word_start++;
        }
        if (!*word_start) break;
        word_end = word_start;
        while (*word_end && (isalnum(*word_end) || *word_end == '_')) {
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

KeywordInfo color_quotes(char* line) {
    KeywordInfo total_info = {0};
    char* current = line;
    
    while (*current && total_info.count < MAX_KEYWORDS) {
        if (*current == '\'') {
            int start = current - line;
            current++;
            
            if (*current == '\\') current += 2;
            else if (*current) current++;
            
            if (*current == '\'') {
                total_info.keywords[total_info.count].start = start;
                total_info.keywords[total_info.count].end = (current - line) + 1;
                total_info.count++;
                current++;
            }
        } 
        else if (*current == '"') {
            int start = current - line;
            current++;
            
            while (*current && *current != '"') {
                if (*current == '\\' && *(current+1)) current += 2;
                else current++;
            }
            
            if (*current == '"') {
                total_info.keywords[total_info.count].start = start;
                total_info.keywords[total_info.count].end = (current - line) + 1;
                total_info.count++;
                current++;
            }
        }
        else {
            current++;
        }
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
        for (int i = 0; i < variables_count; i++) {
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

KeywordInfo check_typedefs(char* line) {
    KeywordInfo total = {0};
    for (size_t i = 0; i < typedefs_count && total.count < MAX_KEYWORDS; i++) {
        KeywordInfo info = check_keyword(line, typedefs[i]);
        for (int k = 0; k < info.count && total.count < MAX_KEYWORDS; k++) {
            total.keywords[total.count++] = info.keywords[k];
        }
    }
    return total;
}

KeywordInfo check_syntax(char* line) {
    KeywordInfo total_info = {0};
    KeywordInfo quotes_info = color_quotes(line);
    KeywordInfo blue_info = check_blue_keywords(line);
    KeywordInfo purple_info = check_purple_keywords(line);
    KeywordInfo function_info = check_functions(line);
    KeywordInfo parentheses_info = color_parentheses(line);
    KeywordInfo variable_info = check_variables(line);
    KeywordInfo comments_info = color_comments(line);
    KeywordInfo typedef_info  = check_typedefs(line);
    KeywordInfo all_infos[] = {blue_info, purple_info, function_info, 
        parentheses_info, variable_info, quotes_info, comments_info, typedef_info};
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < all_infos[i].count && total_info.count < MAX_KEYWORDS; j++) {
            total_info.keywords[total_info.count++] = all_infos[i].keywords[j];
        }
    }
    return total_info;
}