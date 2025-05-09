#include "cc.h"

bool check_parentheses(char text) {
    if (text == '(') {
        insert_char(')');
        return true;
    } else return false;
}

bool check_brackets(char text) {
    if (text == '[') {
        insert_char(']');
        return true;
    } else return false;
}

bool check_braces(char text) {
    if (text == '{') {
        insert_char('}');
        return true;
    } else return false;
}

bool check_comments(char* text) {

}

bool checks(char text) {
    bool check = false;
    if (check_parentheses(text) || check_braces(text) || check_brackets(text)) check = true;
}