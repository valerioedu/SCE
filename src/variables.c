#include "variables.h"
#include "colors.h"
#include <ctype.h>

char** variables = NULL;
size_t variables_capacity = 0;
size_t variables_count = 0;

bool* variables_found = NULL;
bool scan_in_progress = false;

void begin_variable_scan() {
    if (variables) {
        for (size_t i = 0; i < variables_count; i++) {
            free(variables[i]);
        }
        variables_count = 0;
    }

    if (variables_found) free(variables_found);
    variables_found = calloc(variables_capacity, sizeof(bool));
    scan_in_progress = true;
}

void finish_variable_scan() {
    if (!scan_in_progress || !variables_found) return;
    
    size_t i = 0;
    while (i < variables_count) {
        if (!variables_found[i]) {
            free(variables[i]);
            
            for (size_t j = i; j < variables_count - 1; j++) {
                variables[j] = variables[j + 1];
                variables_found[j] = variables_found[j + 1];
            }
            variables_count--;
        } else {
            i++;
        }
    }
    
    free(variables_found);
    variables_found = NULL;
    scan_in_progress = false;
}

void save_variables(char* var) {
    if (variables == NULL) {
        variables_capacity = 16;  // Start with 16
        variables = malloc(variables_capacity * sizeof(char*));
        if (variables == NULL) return;
        memset(variables, 0, variables_capacity * sizeof(char*));
    }
    
    for (size_t i = 0; i < variables_count; i++) {
        if (strcmp(variables[i], var) == 0) {
            return;
        }
    }
    
    if (variables_count >= variables_capacity) {
        size_t new_capacity = variables_capacity * 2;
        char** new_vars = realloc(variables, new_capacity * sizeof(char*));
        if (new_vars) {
            variables = new_vars;
            variables_capacity = new_capacity;
        } else {
            return;
        }
    }
    
    variables[variables_count++] = strdup(var);
}

void detect_variables(char* line) {
    if (inside_multiline_comment) return;
    
    char temp[MAX_COLS];
    char* working_line = line;
    
    char* single_comment = strstr(line, "//");
    if (single_comment) {
        int comment_pos = single_comment - line;
        strncpy(temp, line, comment_pos);
        temp[comment_pos] = '\0';
        working_line = temp;
    }

    char* comment = strstr(line, "/*");
    if (comment) {
        int comment_2p = comment - line;
        strncpy(temp, line, comment_2p);
        temp[comment_2p] = '\0';
        working_line = temp;
    }
    
    if (variables && scan_in_progress) {
        for (size_t i = 0; i < variables_count; i++) {
            if (!variables_found[i]) {
                char* word_start = working_line;
                while ((word_start = strstr(word_start, variables[i])) != NULL) {
                    bool is_start = (word_start == working_line || 
                                   !isalnum(*(word_start-1)) && *(word_start-1) != '_');
                    bool is_end = (!*(word_start + strlen(variables[i])) || 
                                  !isalnum(*(word_start + strlen(variables[i]))) && 
                                  *(word_start + strlen(variables[i])) != '_');
                    
                    if (is_start && is_end) {
                        variables_found[i] = true;
                        break;
                    }
                    word_start++;
                }
            }
        }
    }

    for (int i = 0; i < C_PRIMITIVE_TYPES_SIZE; i++) {
        char* type_pos = working_line;
        
        while ((type_pos = strstr(type_pos, c_primitive_types[i])) != NULL) {
            bool is_start = (type_pos == working_line || 
                           !isalnum(*(type_pos-1)) && *(type_pos-1) != '_');
            bool is_end = (!*(type_pos + strlen(c_primitive_types[i])) || 
                          !isalnum(*(type_pos + strlen(c_primitive_types[i]))) && 
                          *(type_pos + strlen(c_primitive_types[i])) != '_');
            
            if (!is_start || !is_end) {
                type_pos++;
                continue;
            }
            
            char* after_type = type_pos + strlen(c_primitive_types[i]);
            
            while (*after_type && (isspace(*after_type) || *after_type == '*')) after_type++;
            
            while (*after_type && *after_type != ';') {
                while (*after_type && !isalnum(*after_type) && *after_type != '_') after_type++;
                if (!*after_type || *after_type == ';') break;
                
                char* var_start = after_type;
                char* var_end = var_start;
                
                while (*var_end && (isalnum(*var_end) || *var_end == '_')) var_end++;
                
                if (var_end > var_start) {
                    int name_len = var_end - var_start;
                    char* var_name = malloc(name_len + 1);
                    if (var_name) {
                        strncpy(var_name, var_start, name_len);
                        var_name[name_len] = '\0';
                        
                        bool is_keyword = false;
                        for (int j = 0; j < blue_keywords_count; j++) {
                            if (strcmp(var_name, blue_keywords[j]) == 0) {
                                is_keyword = true;
                                break;
                            }
                        }
                        
                        for (int j = 0; j < purple_keywords_count; j++) {
                            if (strcmp(var_name, purple_keywords[j]) == 0) {
                                is_keyword = true;
                                break;
                            }
                        }
                        
                        if (!is_keyword) {
                            save_variables(var_name);
                        }
                        
                        free(var_name);
                    }
                }
                
                after_type = var_end;
                
                while (*after_type && *after_type != ',' && *after_type != ';') after_type++;
                if (*after_type == ',') after_type++;
            }
            
            type_pos = after_type;
        }
    }
}

void cleanup_variables() {
    if (variables) {
        for (size_t i = 0; i < variables_count; i++) {
            free(variables[i]);
        }
        free(variables);
        variables = NULL;
    }
}