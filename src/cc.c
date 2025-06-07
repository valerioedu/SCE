#include "cc.h"

bool check_parentheses(char text) {
    if (!config.parenthesis_autocomplete) return false;
    if (text == '(') {
        insert_char(')');
        return true;
    } else return false;
}

bool check_brackets(char text) {
    if (!config.parenthesis_autocomplete) return false;
    if (text == '[') {
        insert_char(']');
        return true;
    } else return false;
}

bool check_braces(char text) {
    if (!config.parenthesis_autocomplete) return false;
    if (text == '{') {
        insert_char('}');
        return true;
    } else return false;
}

bool check_quotations(char text) {
    if (!config.quotations_autocomplete) return false;
    if (text == '"') {
        insert_char('"');
        return true;
    }
    else if (text == '\'') {
        insert_char('\'');
        return true;
    }
    else return false;
}

bool checks(char text) {
     return check_parentheses(text) || check_braces(text) ||
     check_brackets(text) || check_quotations(text);
}