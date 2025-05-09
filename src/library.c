#include "library.h"

void insert_char(char c) {
    save_undo_state();
    if (current_col < MAX_COLS - 1) {
        memmove(&lines[current_line][current_col + 1], &lines[current_line][current_col], strlen(lines[current_line]) - current_col + 1);
        lines[current_line][current_col] = c;
        current_col++;
    }
}

void tab() {
    if (current_col % TABS_SIZE == 0) for (int i = 0; i < TABS_SIZE; i++) insert_char(' ');
    else while (current_col % TABS_SIZE != 0) insert_char(' ');
}

void transcribe_to_text() {
    int text_index = 0;
    for (int i = 0; i < line_count; i++) {
        int line_length = strlen(lines[i]);
        if (text_index + line_length + 1 >= MAX_LINES * MAX_COLS) {
            break;
        }
        strcpy(&text[text_index], lines[i]);
        text_index += line_length;
        
        if (i < line_count - 1) {
            text[text_index] = '\n';
            text_index++;
        }
    }
    text[text_index] = '\0';
}