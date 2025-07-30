#ifndef _WIN32
#include "languages.h"

#define CS_PATTERN_COUNT 15

Pattern CsPatterns[] = {
    // Control flow
    {"\\b(if|else|switch|case|default|for|foreach|while|do|break|continue|return|goto|try|catch|finally|throw|yield|await)\\b", 2},
    
    // Types and keywords
    {"\\b(class|struct|interface|enum|delegate|namespace|using|public|private|protected|internal|static|readonly|const|virtual|override|abstract|sealed|partial|async|ref|out|in|params|this|base|new|typeof|sizeof|is|as|null|true|false)\\b", 1},
    
    // Built-in types
    {"\\b(int|long|short|byte|sbyte|uint|ulong|ushort|float|double|decimal|bool|char|string|object|void|var|dynamic)\\b", 1},
    
    // Numbers
    {"\\b[0-9]+\\.?[0-9]*[fFdDmM]?\\b", 8},
    {"\\b0[xX][0-9a-fA-F]+[lLuU]*\\b", 8},
    
    // Comments
    {"//.*", 7},
    {"/\\*[^*]*\\*+([^/*][^*]*\\*+)*/", 7},
    
    // Strings
    {"\"([^\"\\\\]|\\\\.)*\"", 6},
    {"'([^'\\\\]|\\\\.)*'", 6},
    {"@\"([^\"]|\"\")*\"", 6}, // Verbatim strings
    {"\\$\"([^\"\\\\]|\\\\.)*\"", 6}, // Interpolated strings
    
    // Attributes
    {"\\[[^\\]]*\\]", 8},
    
    // Parentheses
    {"[(){}\\[\\]]", 4},
    
    // Functions/methods
    {"\\b[a-zA-Z_][a-zA-Z0-9_]*\\(", 3},
    
    // Preprocessor
    {"#[a-zA-Z]+", 8}
};

#define BASH_PATTERN_COUNT 12

Pattern BashPatterns[] = {
    // Control flow
    {"\\b(if|then|else|elif|fi|for|while|until|do|done|case|esac|function|return|break|continue|exit)\\b", 2},
    
    // Keywords and builtins
    {"\\b(echo|printf|read|cd|pwd|ls|cp|mv|rm|mkdir|chmod|chown|grep|sed|awk|sort|uniq|head|tail|cat|less|more|find|xargs|pipe|export|source|alias|unalias|history|jobs|bg|fg|nohup|kill|ps|top|df|du|mount|umount|tar|gzip|gunzip|wget|curl|ssh|scp|rsync)\\b", 1},
    
    // Variables
    {"\\$[a-zA-Z_][a-zA-Z0-9_]*", 3},
    {"\\$\\{[^}]+\\}", 3},
    {"\\$[0-9#@*?$!-]", 3},
    
    // Numbers
    {"\\b[0-9]+\\b", 8},
    
    // Comments
    {"#.*", 7},
    
    // Strings
    {"\"([^\"\\\\]|\\\\.)*\"", 6},
    {"'[^']*'", 6},
    {"`[^`]*`", 6}, // Command substitution
    
    // Operators and redirections
    {"[(){}\\[\\]|&;<>]", 4},
    
    // Command substitution with $()
    {"\\$\\([^)]*\\)", 6}
};

#define PS_PATTERN_COUNT 14

Pattern PsPatterns[] = {
    // Control flow
    {"\\b(if|elseif|else|switch|for|foreach|while|do|until|break|continue|return|try|catch|finally|throw|trap)\\b", 2},
    
    // Keywords
    {"\\b(function|filter|workflow|class|enum|param|begin|process|end|dynamicparam|using|namespace|module|import-module|export-modulemember)\\b", 1},
    
    // Types
    {"\\[(int|long|string|bool|double|float|decimal|char|byte|array|hashtable|psobject|datetime|regex|xml|scriptblock|type)\\]", 1},
    
    // Variables
    {"\\$[a-zA-Z_][a-zA-Z0-9_]*", 3},
    {"\\$\\{[^}]+\\}", 3},
    {"\\$[?^$_]", 3},
    
    // Numbers
    {"\\b[0-9]+\\.?[0-9]*[lLdD]?\\b", 8},
    {"\\b0[xX][0-9a-fA-F]+\\b", 8},
    
    // Comments
    {"#.*", 7},
    {"<#[^#]*#>", 7}, // Block comments
    
    // Strings
    {"\"([^\"\\\\]|\\\\.)*\"", 6},
    {"'[^']*'", 6},
    
    // Parentheses and operators
    {"[(){}\\[\\]|&;]", 4},
    
    // Cmdlets (PowerShell commands)
    {"\\b[A-Z][a-z]+-[A-Z][a-zA-Z]*\\b", 3}
};

#define GO_PATTERN_COUNT 14

Pattern GoPatterns[] = {
    // Control flow
    {"\\b(if|else|switch|case|default|for|range|break|continue|return|goto|defer|go|select)\\b", 2},
    
    // Keywords and types
    {"\\b(package|import|var|const|type|func|struct|interface|map|chan|len|cap|make|new|append|copy|delete|panic|recover|close)\\b", 1},
    
    // Built-in types
    {"\\b(bool|byte|rune|int|int8|int16|int32|int64|uint|uint8|uint16|uint32|uint64|uintptr|float32|float64|complex64|complex128|string|error)\\b", 1},
    
    // Constants
    {"\\b(true|false|nil|iota)\\b", 1},
    
    // Numbers
    {"\\b[0-9]+\\.?[0-9]*([eE][+-]?[0-9]+)?\\b", 8},
    {"\\b0[xX][0-9a-fA-F]+\\b", 8},
    {"\\b0[0-7]+\\b", 8},
    
    // Comments
    {"//.*", 7},
    {"/\\*[^*]*\\*+([^/*][^*]*\\*+)*/", 7},
    
    // Strings
    {"\"([^\"\\\\]|\\\\.)*\"", 6},
    {"'([^'\\\\]|\\\\.)*'", 6},
    {"`[^`]*`", 6}, // Raw strings
    
    // Parentheses
    {"[(){}\\[\\]]", 4},
    {"\\b[a-zA-Z_][a-zA-Z0-9_]*\\(", 3}
};

#define RUST_PATTERN_COUNT 17

Pattern RustPatterns[] = {
    // Control flow
    {"\\b(if|else|match|for|while|loop|break|continue|return|yield)\\b", 2},
    
    // Keywords
    {"\\b(fn|let|mut|const|static|struct|enum|impl|trait|type|use|mod|pub|crate|extern|unsafe|async|await|move|ref|dyn|where|as|self|Self|super)\\b", 1},
    
    // Built-in types
    {"\\b(i8|i16|i32|i64|i128|isize|u8|u16|u32|u64|u128|usize|f32|f64|bool|char|str|String|Vec|Option|Result)\\b", 1},
    
    // Constants
    {"\\b(true|false|None|Some|Ok|Err)\\b", 1},
    
    // Macros
    {"\\b[a-zA-Z_][a-zA-Z0-9_]*!", 8},
    
    // Lifetimes
    {"'[a-zA-Z_][a-zA-Z0-9_]*\\b", 8},
    
    // Numbers
    {"\\b[0-9]+\\.?[0-9]*([eE][+-]?[0-9]+)?[fF]?(32|64)?\\b", 8},
    {"\\b0[xX][0-9a-fA-F]+[uUiI]?(8|16|32|64|128|size)?\\b", 8},
    {"\\b0[oO][0-7]+[uUiI]?(8|16|32|64|128|size)?\\b", 8},
    {"\\b0[bB][01]+[uUiI]?(8|16|32|64|128|size)?\\b", 8},
    
    // Comments
    {"//.*", 7},
    {"/\\*[^*]*\\*+([^/*][^*]*\\*+)*/", 7},
    
    // Strings
    {"\"([^\"\\\\]|\\\\.)*\"", 6},
    {"'([^'\\\\]|\\\\.)*'", 6},
    {"r#*\"[^\"]*\"#*", 6}, // Raw strings
    
    // Parentheses
    {"[(){}\\[\\]]", 4},
    {"\\b[a-zA-Z_][a-zA-Z0-9_]*\\(", 3}
};

#define JAVA_PATTERN_COUNT 14

Pattern JavaPatterns[] = {
    // Control flow
    {"\\b(if|else|switch|case|default|for|while|do|break|continue|return|try|catch|finally|throw|throws|synchronized)\\b", 2},
    
    // Keywords and modifiers
    {"\\b(class|interface|enum|extends|implements|package|import|public|private|protected|static|final|abstract|native|strictfp|transient|volatile|synchronized|this|super|new|instanceof)\\b", 1},
    
    // Built-in types
    {"\\b(boolean|byte|char|short|int|long|float|double|void|String|Object)\\b", 1},
    
    // Constants
    {"\\b(true|false|null)\\b", 1},
    
    // Numbers
    {"\\b[0-9]+\\.?[0-9]*[fFdDlL]?\\b", 8},
    {"\\b0[xX][0-9a-fA-F]+[lL]?\\b", 8},
    {"\\b0[0-7]+[lL]?\\b", 8},
    
    // Comments
    {"//.*", 7},
    {"/\\*[^*]*\\*+([^/*][^*]*\\*+)*/", 7},
    
    // Strings
    {"\"([^\"\\\\]|\\\\.)*\"", 6},
    {"'([^'\\\\]|\\\\.)*'", 6},
    
    // Annotations
    {"@[a-zA-Z_][a-zA-Z0-9_]*", 8},
    
    // Parentheses
    {"[(){}\\[\\]]", 4},
    
    // Functions/methods
    {"\\b[a-zA-Z_][a-zA-Z0-9_]*[ \t]*\\(", 3}
};

#define JS_PATTERN_COUNT 15

Pattern JsPatterns[] = {
    // Control flow
    {"\\b(if|else|switch|case|default|for|while|do|break|continue|return|try|catch|finally|throw|with)\\b", 2},
    
    // Keywords
    {"\\b(var|let|const|function|class|extends|import|export|from|as|default|new|this|super|typeof|instanceof|in|of|delete|void|async|await|yield|static|get|set)\\b", 1},
    
    // Built-in objects and types
    {"\\b(Object|Array|String|Number|Boolean|Date|RegExp|Function|Error|Promise|Symbol|Map|Set|WeakMap|WeakSet|Proxy|Reflect|JSON|Math|console|window|document|undefined|Infinity|NaN)\\b", 1},
    
    // Constants
    {"\\b(true|false|null|undefined)\\b", 1},
    
    // Numbers
    {"\\b[0-9]+\\.?[0-9]*([eE][+-]?[0-9]+)?\\b", 8},
    {"\\b0[xX][0-9a-fA-F]+\\b", 8},
    {"\\b0[oO][0-7]+\\b", 8},
    {"\\b0[bB][01]+\\b", 8},
    
    // Comments
    {"//.*", 7},
    {"/\\*[^*]*\\*+([^/*][^*]*\\*+)*/", 7},
    
    // Strings and template literals
    {"\"([^\"\\\\]|\\\\.)*\"", 6},
    {"'([^'\\\\]|\\\\.)*'", 6},
    {"`([^`\\\\]|\\\\.)*`", 6}, // Template literals
    
    // Regular expressions
    {"/([^/\\\\\\n]|\\\\.)+/[gimsuvy]*", 6},
    
    // Parentheses
    {"[(){}\\[\\]]", 4},
    {"\\b[a-zA-Z_][a-zA-Z0-9_]*\\(", 3}
};


int find_csharp_matches(const char* line, MatchInfo* matches, int max_matches) {
    return find_matches_generic(line, matches, max_matches, CsPatterns, CS_PATTERN_COUNT);
}

int find_bash_matches(const char* line, MatchInfo* matches, int max_matches) {
    return find_matches_generic(line, matches, max_matches, BashPatterns, BASH_PATTERN_COUNT);
}

int find_powershell_matches(const char* line, MatchInfo* matches, int max_matches) {
    return find_matches_generic(line, matches, max_matches, PsPatterns, PS_PATTERN_COUNT);
}

int find_go_matches(const char* line, MatchInfo* matches, int max_matches) {
    return find_matches_generic(line, matches, max_matches, GoPatterns, GO_PATTERN_COUNT);
}

int find_rust_matches(const char* line, MatchInfo* matches, int max_matches) {
    return find_matches_generic(line, matches, max_matches, RustPatterns, RUST_PATTERN_COUNT);
}

int find_java_matches(const char* line, MatchInfo* matches, int max_matches) {
    return find_matches_generic(line, matches, max_matches, JavaPatterns, JAVA_PATTERN_COUNT);
}

int find_javascript_matches(const char* line, MatchInfo* matches, int max_matches) {
    return find_matches_generic(line, matches, max_matches, JsPatterns, JS_PATTERN_COUNT);
}

int find_matches_generic(const char* line, MatchInfo* matches, int max_matches, Pattern* patterns, int pattern_count) {
    int match_count = 0;
    
    for (int pattern_idx = 0; pattern_idx < pattern_count && match_count < max_matches; pattern_idx++) {
        regex_t regex;
        regmatch_t match;
        
        if (regcomp(&regex, patterns[pattern_idx].regex, REG_EXTENDED) != 0) {
            continue;
        }
        
        const char* search_start = line;
        int offset = 0;
        
        while (regexec(&regex, search_start, 1, &match, 0) == 0 && match_count < max_matches) {
            if (strstr(patterns[pattern_idx].regex, "\\(") != NULL) {
                matches[match_count].start = offset + match.rm_so;
                matches[match_count].end = offset + match.rm_eo - 1;
                matches[match_count].color = patterns[pattern_idx].color;
            } else {
                matches[match_count].start = offset + match.rm_so;
                matches[match_count].end = offset + match.rm_eo;
                matches[match_count].color = patterns[pattern_idx].color;
            }
            match_count++;
            
            offset += match.rm_eo;
            search_start += match.rm_eo;
            
            if (match.rm_eo == 0) {
                offset++;
                search_start++;
            }
        }
        
        regfree(&regex);
    }
    
    return match_count;
}

#define PY_PATTERN_COUNT 12

Pattern PyPatterns[] = {     
    {"\\b(if|elif|else|for|while|break|continue|return|try|except|finally|raise|with|as|pass|lambda|yield|True|False|None|and|or|not|is|in)\\b", 2},
    
    {"\\b(def|class|import|from|global|nonlocal|async|await)\\b", 1},
    
    {"\\b[0-9]+\\.?[0-9]*\\b", 8},
    {"\\b0[xX][0-9a-fA-F]+\\b", 8},
    {"#.*", 7},
    {"\"([^\"\\\\]|\\\\.)*\"", 6},
    {"'([^'\\\\]|\\\\.)*'", 6},
    {"\"\"\"[^\"]*\"\"\"", 6},
    {"'''[^']*'''", 6},
    {"[fF][\"']([^\"'\\\\]|\\\\.)*[\"']", 6},
    
    {"[()]", 4},
    {"\\b[a-zA-Z_][a-zA-Z0-9_]*[ \t]*\\(", 3}
};

int find_python_matches(const char* line, MatchInfo* matches, int max_matches) {
    int match_count = 0;
    
    for (int pattern_idx = 0; pattern_idx < PY_PATTERN_COUNT && match_count < max_matches; pattern_idx++) {
        regex_t regex;
        regmatch_t match;
        
        if (regcomp(&regex, PyPatterns[pattern_idx].regex, REG_EXTENDED) != 0) {
            continue;
        }
        
        const char* search_start = line;
        int offset = 0;
        
        while (regexec(&regex, search_start, 1, &match, 0) == 0 && match_count < max_matches) {
            if (pattern_idx == 11) {
                matches[match_count].start = offset + match.rm_so;
                matches[match_count].end = offset + match.rm_eo - 1;
                matches[match_count].color = PyPatterns[pattern_idx].color;
            } else {
                matches[match_count].start = offset + match.rm_so;
                matches[match_count].end = offset + match.rm_eo;
                matches[match_count].color = PyPatterns[pattern_idx].color;
            }
            match_count++;
            
            offset += match.rm_eo;
            search_start += match.rm_eo;
            
            if (match.rm_eo == 0) {
                offset++;
                search_start++;
            }
        }
        
        regfree(&regex);
    }
    
    return match_count;
}

int get_color_at_position(MatchInfo* matches, int match_count, int position) {
    for (int i = match_count - 1; i >= 0; i--) {
        if (position >= matches[i].start && position < matches[i].end) {
            return matches[i].color;
        }
    }
    return -1;
}
#endif