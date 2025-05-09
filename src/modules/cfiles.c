#include "cfiles.h"

const char* c_primitive_types[] = {
    "void", "int", "char", "short", "long", "float", "double",
    "size_t", "int8_t", "int16_t", "int32_t", "int64_t",
    "uint8_t", "uint16_t", "uint32_t", "uint64_t",
};

char* definitions[MAX_DEFINITIONS];

void save_definition(char* def) {
    for (int i = 0; i < MAX_DEFINITIONS; i++) {
        if (definitions[i] == NULL) {
            definitions[i] = strdup(def);
            return;
        } else if (strcmp(definitions[i], def) == 0) {
            return;
        }
    }
}

void detect_defines(char* line) {
    char line_copy[MAX_COLS];
    strncpy(line_copy, line, MAX_COLS - 1);
    line_copy[MAX_COLS - 1] = '\0';
    char* word = strtok(line_copy, " \t\n");
    while (word != NULL) {
        bool is_defined = false;
        if (strcmp(word, "#define") == 0) {
            is_defined = true;
        }
        if (is_defined) {
            word = strtok(NULL, " \t\n;,");
            if (word != NULL) {
                save_definition(word);
            }
        }    
        word = strtok(NULL, " \t\n");
    }
}