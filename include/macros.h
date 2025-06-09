#ifndef MACROS_H
#define MACROS_H

void ctrl_f();
void save_undo_state();
void ctrl_z();
void cleanup_undo_history();
void ctrl_left_arrow(char* line);
void ctrl_right_arrow(char* line);
void save_cursor(int line, int col);
void ctrl_up();
void ctrl_down();

#endif