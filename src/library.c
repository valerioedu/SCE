#include "library.h"
#include "variables.h"

bool open_file_browser = false;

void insert_char(char c) {
    if (current_col < MAX_COLS - 1 && strlen(lines[current_line]) < MAX_COLS - 1) {
        if (c == '\t' && current_col > MAX_COLS - TABS_SIZE) return;
        else if (c == '\t' && current_col < MAX_COLS - TABS_SIZE) {
            save_undo_state();
            if (config.expandtab) {
                int spaces = TABS_SIZE - (current_col % TABS_SIZE);
                for (int i = 0; i < spaces; i++) {
                    memmove(&lines[current_line][current_col + 1], &lines[current_line][current_col], strlen(lines[current_line]) - current_col + 1);
                    lines[current_line][current_col] = ' ';
                    current_col++;
                }
            } else {        // TODO: Implement better \t handling, now copy expandtab
                int spaces = TABS_SIZE - (current_col % TABS_SIZE);
                for (int i = 0; i < spaces; i++) {
                    memmove(&lines[current_line][current_col + 1], &lines[current_line][current_col], strlen(lines[current_line]) - current_col + 1);
                    lines[current_line][current_col] = ' ';
                    current_col++;
                }
            } 
        } else {
            save_undo_state();
            memmove(&lines[current_line][current_col + 1], &lines[current_line][current_col], strlen(lines[current_line]) - current_col + 1);
            lines[current_line][current_col] = c;
            current_col++;
        }
    }
    rescan_line_for_declarations(current_line);
}

void ensure_text_capacity(size_t needed_size) {
    if (text_capacity >= needed_size) return;
    
    size_t new_capacity = needed_size + 1024;
    char *new_text = realloc(text, new_capacity);
    
    if (!new_text) return;
    
    text = new_text;
    text_capacity = new_capacity;
}

void cleanup_text() {
    if (text) {
        free(text);
        text = NULL;
        text_capacity = 0;
    }
}

void transcribe_to_text() {
    size_t total_size = 0;
    for (int i = 0; i < line_count; i++) {
        total_size += strlen(lines[i]);
        if (i < line_count - 1) total_size++;
    }
    
    ensure_text_capacity(total_size + 1); // +1 for null terminator
    
    int text_index = 0;
    for (int i = 0; i < line_count; i++) {
        int line_length = strlen(lines[i]);
        strcpy(&text[text_index], lines[i]);
        text_index += line_length;
        
        if (i < line_count - 1) {
            text[text_index] = '\n';
            text_index++;
        }
    }
    text[text_index] = '\0';
}