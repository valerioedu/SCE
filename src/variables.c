#include "variables.h"

char** variables = NULL;
size_t variables_capacity = 0;
size_t variables_count = 0;

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
    char line_copy[MAX_COLS];
    strncpy(line_copy, line, MAX_COLS - 1);
    line_copy[MAX_COLS - 1] = '\0';

    char* word = strtok(line_copy, " \t\n");
    while (word != NULL) {
        bool is_keyword = false;
        for (int i = 0; i < C_PRIMITIVE_TYPES_SIZE; i++) {
            if (strcmp(word, c_primitive_types[i]) == 0) {
                is_keyword = true;
                break;
            }
        }
        
        if (is_keyword) {
            word = strtok(NULL, " \t\n;,");
            if (word != NULL) {
                save_variables(word);
            }
        }
        
        word = strtok(NULL, " \t\n");
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