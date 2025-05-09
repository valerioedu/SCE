#include "variables.h"

char* variables[MAX_VARIABLES];

void save_variables(char* var) {
    for (int i = 0; i < MAX_VARIABLES; i++) {
        if (variables[i] == NULL) {
            variables[i] = strdup(var);
            return;
        } else if (strcmp(variables[i], var) == 0) {
            return;
        }
    }
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