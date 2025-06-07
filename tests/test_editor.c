#include <check.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "macros.h"
#include "variables.h"
#include "editorfile.h"
#include "colors.h"
#include "sceconfig.h"
#include "arg.h"
#include "cc.h"
#include "cfiles.h"
#include "console.h"
#include "git.h"
#include "pages.h"

/*
*   +----------------------------------------+
*   | TODO: seperate tests in multiple files |
*   +----------------------------------------+
*/

EditorConfig config = {0};
int horizontal_offset = 0;

UndoState undo_history[MAX_UNDO] = {0};
int undo_count = 0;
int undo_position = 0;
char** lines = NULL;
size_t lines_capacity = 0;
size_t count = 1;
int line_count = 1;
int current_line = 0;
int current_col = 0;
int start_line = 0;
char file_name[512] = {0};
char text[MAX_LINES * MAX_COLS] = {0};


void init_lines() {
    lines_capacity = 100;  // Initial capacity of 100 lines
    lines = malloc(lines_capacity * sizeof(char*));
    if (!lines) {
        endwin();
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    
    for (size_t i = 0; i < lines_capacity; i++) {
        lines[i] = malloc(MAX_COLS);
        if (!lines[i]) {
            endwin();
            fprintf(stderr, "Memory allocation failed\n");
            exit(1);
        }
        lines[i][0] = '\0';
    }
}

void ensure_lines_capacity(size_t needed_lines) {
    if (needed_lines <= lines_capacity) return;
    
    size_t new_capacity = lines_capacity * 2;
    while (new_capacity < needed_lines) new_capacity *= 2;
    
    char** new_lines = realloc(lines, new_capacity * sizeof(char*));
    if (!new_lines) return;
    
    lines = new_lines;
    
    for (size_t i = lines_capacity; i < new_capacity; i++) {
        lines[i] = malloc(MAX_COLS);
        if (!lines[i]) return;
        lines[i][0] = '\0';
    }
    
    lines_capacity = new_capacity;
}

void cleanup_lines() {
    if (!lines) return;
    
    for (size_t i = 0; i < lines_capacity; i++) {
        free(lines[i]);
    }
    free(lines);
    lines = NULL;
}

void update_screen_content(int start_line) {}

void update_status_bar() {}

void display_lines() {}

void apply_resize() {}

void editor() {}

void init_editor() {}

int mainc_main(int argc, char* argv[]) {    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage();
            exit(0);
        } else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0) {
            print_version();
            exit(0);
        }
    }
    init_editor();
    load_config(&config);
    init_lines();
    if (open_file_browser) {
        filesystem(current_path);
    }
    args(argc, argv);

    for (int i = 0; i < MAX_UNDO; i++) {
        undo_history[i].lines = NULL;
        undo_history[i].line_count = 0;
    }

    for (int i = 0; i < line_count; i++) detect_variables(lines[i]);

    display_lines();
    update_screen_content(0);

    while (1) editor();

    cleanup_lines();
    cleanup_undo_history();
    cleanup_variables();
    endwin();
    return 0;
}

void setup(void) {
    init_lines();
    current_line = 0;
    current_col = 0;
    line_count = 1;
    file_name[0] = '\0';
    undo_count = 0;
    undo_position = 0;
    horizontal_offset = 0;
    
    for (int i = 0; i < lines_capacity; i++) {
        lines[i][0] = '\0';
    }
}

void teardown(void) {
    cleanup_lines();
    cleanup_undo_history();
}

START_TEST(test_insert_char) {
    insert_char('a');
    ck_assert_str_eq(lines[0], "a");
    ck_assert_int_eq(current_col, 1);
    
    current_col = 0;
    insert_char('b');
    ck_assert_str_eq(lines[0], "ba");
    ck_assert_int_eq(current_col, 1);

    insert_char('c');
    ck_assert_str_eq(lines[0], "bca");
    ck_assert_int_eq(current_col, 2);
    
    current_col = 3;
    insert_char('d');
    ck_assert_str_eq(lines[0], "bcad");
    ck_assert_int_eq(current_col, 4);
    
    insert_char('!');
    ck_assert_str_eq(lines[0], "bcad!");
    
    insert_char(' ');
    ck_assert_str_eq(lines[0], "bcad! ");
}
END_TEST

// Emulates backspace
START_TEST(test_backspace) {
    strcpy(lines[0], "hello");
    current_col = 5;
    
    memmove(&lines[current_line][current_col - 1], &lines[current_line][current_col], 
            strlen(lines[current_line]) - current_col + 1);
    current_col--;
    ck_assert_str_eq(lines[0], "hell");
    ck_assert_int_eq(current_col, 4);
    
    current_col = 2;
    memmove(&lines[current_line][current_col - 1], &lines[current_line][current_col], 
            strlen(lines[current_line]) - current_col + 1);
    current_col--;
    ck_assert_str_eq(lines[0], "hll");
    ck_assert_int_eq(current_col, 1);
    
    current_col = 0;
    if (current_col > 0) {
        memmove(&lines[current_line][current_col - 1], &lines[current_line][current_col], 
                strlen(lines[current_line]) - current_col + 1);
        current_col--;
    }
    ck_assert_str_eq(lines[0], "hll");
    ck_assert_int_eq(current_col, 0);
    
    line_count = 2;
    strcpy(lines[0], "first");
    strcpy(lines[1], "second");
    current_line = 1;
    current_col = 0;
    
    if (current_col == 0 && current_line > 0) {
        current_col = strlen(lines[current_line - 1]);
        if (current_col + strlen(lines[current_line]) < MAX_COLS) {
            strcat(lines[current_line - 1], lines[current_line]);

            for (int i = current_line; i < line_count - 1; i++) {
                strcpy(lines[i], lines[i + 1]);
            }
            
            line_count--;
            current_line--;
        }
    }
    
    ck_assert_str_eq(lines[0], "firstsecond");
    ck_assert_int_eq(line_count, 1);
    ck_assert_int_eq(current_line, 0);
    ck_assert_int_eq(current_col, 5);
}
END_TEST

START_TEST(test_line_handling) {
    strcpy(lines[0], "This is a test line");
    current_col = 9;
    
    ensure_lines_capacity(line_count + 1);
    for (int i = line_count; i > current_line + 1; i--) {
        strcpy(lines[i], lines[i-1]);
    }
    
    strncpy(lines[current_line + 1], &lines[current_line][current_col], MAX_COLS - 1);
    lines[current_line + 1][MAX_COLS - 1] = '\0';
    lines[current_line][current_col] = '\0';
    
    line_count++;
    current_line++;
    current_col = 0;
    
    ck_assert_int_eq(line_count, 2);
    ck_assert_str_eq(lines[0], "This is a");
    ck_assert_str_eq(lines[1], " test line");
    ck_assert_int_eq(current_line, 1);
    ck_assert_int_eq(current_col, 0);
    
    current_line = 0;
    strcpy(lines[0], "    indented");
    
    int indent = 0;
    while (indent < strlen(lines[current_line]) && lines[current_line][indent] == ' ') {
        indent++;
    }
    
    ck_assert_int_eq(indent, 4);
}
END_TEST

START_TEST(test_cursor_movement) {
    strcpy(lines[0], "Line 1");
    strcpy(lines[1], "Line 2");
    line_count = 2;
    current_line = 0;
    current_col = 0;
    
    current_col++;
    ck_assert_int_eq(current_col, 1);
    
    current_col = strlen(lines[current_line]);
    ck_assert_int_eq(current_col, 6);
    
    current_line++;
    ck_assert_int_eq(current_line, 1);
    
    current_col = 0;
    ck_assert_int_eq(current_col, 0);
    
    current_line--;
    ck_assert_int_eq(current_line, 0);
    
    current_col = strlen(lines[current_line]) + 5;
    if (current_col > strlen(lines[current_line])) {
        current_col = strlen(lines[current_line]);
    }
    ck_assert_int_eq(current_col, 6);
}
END_TEST

void restore_undo_state() {
    if (undo_position <= 0) return;
    undo_position--;
}

START_TEST(test_undo) {
    strcpy(lines[0], "Initial");
    current_line = 0;
    current_col = 7;
    
    undo_history[0].line_count = line_count;
    undo_history[0].cursor_line = current_line;
    undo_history[0].cursor_col = current_col;
    
    if (undo_history[0].lines == NULL) {
        undo_history[0].lines = malloc(line_count * sizeof(char*));
        for (int i = 0; i < line_count; i++) {
            undo_history[0].lines[i] = malloc(MAX_COLS);
            strcpy(undo_history[0].lines[i], lines[i]);
        }
    }
    
    undo_count = 1;
    undo_position = 1;
    
    strcpy(lines[0], "Modified");
    current_col = 3;
    
    restore_undo_state();
    
    ck_assert_str_eq(lines[0], "Initial");
    ck_assert_int_eq(current_line, 0);
    ck_assert_int_eq(current_col, 7);
}
END_TEST

START_TEST(test_memory_management) {
    size_t initial_capacity = lines_capacity;
    
    ensure_lines_capacity(10000);
    
    if (initial_capacity < 10000) {
        ck_assert(lines_capacity >= 10000);
    }
    
    for (int i = 0; i < 10000; i++) {
        sprintf(lines[i], "Line %d", i + 1);
    }
    line_count = 10000;
    
    ck_assert_str_eq(lines[0], "Line 1");
    ck_assert_str_eq(lines[999], "Line 1000");
    ck_assert_str_eq(lines[4999], "Line 5000");
    ck_assert_str_eq(lines[9999], "Line 10000");
    
    strcpy(lines[5000], "Modified line");
    ck_assert_str_eq(lines[5000], "Modified line");
    
    strcpy(lines[9999], "Last line modified");
    ck_assert_str_eq(lines[9999], "Last line modified");
    
    ck_assert_int_eq(line_count, 10000);
    ck_assert(lines_capacity >= 10000);
}
END_TEST

START_TEST(test_file_operations) {
    FILE* f = fopen("test_file.txt", "w");
    fprintf(f, "Line 1\nLine 2\nLine 3\n");
    fclose(f);
    
    for (int i = 0; i < line_count; i++) {
        lines[i][0] = '\0';
    }
    line_count = 1;
    int old_line_count = line_count;
    
    strcpy(file_name, "test_file.txt");
    load_file(file_name);
    
    ck_assert_int_eq(line_count, 3);
    ck_assert_int_ne(old_line_count, line_count);
    ck_assert_str_eq(lines[0], "Line 1");
    ck_assert_str_eq(lines[1], "Line 2");
    ck_assert_str_eq(lines[2], "Line 3");
    
    remove("test_file.txt");
}
END_TEST

START_TEST(test_syntax_highlighting) {
    strcpy(lines[0], "int main() {");
    
    KeywordInfo blue_info = check_blue_keywords(lines[0]);
    KeywordInfo purple_info = check_purple_keywords(lines[0]);
    
    ck_assert_int_gt(blue_info.count, 0);
    ck_assert_int_eq(blue_info.keywords[0].start, 0);
    ck_assert_int_eq(blue_info.keywords[0].end, 3);
    
    strcpy(lines[0], "// This is a comment");
    KeywordInfo comments_info = color_comments(lines[0]);
    
    ck_assert_int_gt(comments_info.count, 0);
}
END_TEST

Suite* editor_suite(void) {
    Suite* s;
    TCase* tc_core, *tc_text, *tc_file, *tc_syntax;

    s = suite_create("Editor");
    
    tc_core = tcase_create("Core");
    tcase_add_checked_fixture(tc_core, setup, teardown);
    tcase_add_test(tc_core, test_insert_char);
    tcase_add_test(tc_core, test_backspace);
    tcase_add_test(tc_core, test_cursor_movement);
    suite_add_tcase(s, tc_core);
    
    tc_text = tcase_create("Text");
    tcase_add_checked_fixture(tc_text, setup, teardown);
    tcase_add_test(tc_text, test_line_handling);
    tcase_add_test(tc_text, test_undo);
    tcase_add_test(tc_text, test_memory_management);
    suite_add_tcase(s, tc_text);
    
    tc_file = tcase_create("File");
    tcase_add_checked_fixture(tc_file, setup, teardown);
    tcase_add_test(tc_file, test_file_operations);
    suite_add_tcase(s, tc_file);
    
    tc_syntax = tcase_create("Syntax");
    tcase_add_checked_fixture(tc_syntax, setup, teardown);
    tcase_add_test(tc_syntax, test_syntax_highlighting);
    suite_add_tcase(s, tc_syntax);

    return s;
}

int main() {
    int number_failed;
    Suite* s;
    SRunner* sr;

    s = editor_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_VERBOSE);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    
    return (number_failed == 0) ? 0 : 1;
}