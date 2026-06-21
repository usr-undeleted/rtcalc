#ifndef UTILITIES_H
#define UTILITIES_H

// functions that do one small task go here, or helper functions

#include "definitions.h"
#include <stdlib.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

extern int retCode;
extern struct termios backup;

// show help message and quits
static inline void helpMenu(char *error, int ret) {
    printf("\e[1mReal-time calculation tool (rtcalc)\e[0m\n"

        "\e[3mLegal and basic info:\e[0m\n"
        "- Licensed under the GNU GPL-3.0 license. Open-source, and free, forever.\n"
        "- Source code hosted under Github (https://github.com/usr-undeleted/rtcalc).\n"
        "- Current version: \e[1m%s\e[0m\n"
        "- Made with love, by Undeleted. <3\n"
        "- Functions, operators and basic syntax can be found in this program's manual.\n"
        "\n"

        "\e[3mUsage:\e[0m\n"
        "- This tool, of course, follows basic math principles.\n"
        "\e[1mBasic examples:\e[0m\n"
        "\e[4m10 + 2 * 3\e[0m\n"
        "\e[4m2 ^ (10 * (40 / 2))\e[0m\n\n"

        "\e[3mDetails:\e[0m\n"
        "- Invalid input will lead to an error, preventing calculation.\n"
        "- Input is limited to %zu characters.\n"
        "- Result size is limited to %zu characters.\n"
        "\n"

        "\e[3mAdditional arguments:\e[0m\n"
        "- \e[1m\"help\"\e[0m: Show this menu.\n"
        "- \e[1m\"prompt=<prompt>\"\e[0m: Define a custom prompt before startup (space not included).\n"
        "- \e[1m\"syntax-highlighting\"\e[0m: Enable syntax highlighting on the input.\n"
        "- \e[1m\"precision=<num>\"\e[0m: Define what precision to show results in. Defaults to 6.\n"
        "- \e[1m\"define:<name>=<num>\"\e[0m: Create an immutable variable acessible during runtime. Definition of value may be a formula supported by the program, and may include variables defined earlier.\n"
        "%s"
        ,
        VERSION, BUFFER_SIZE - 1, RESULT_SIZE - 1, error != NULL ? error : "");
    exit(ret);
}

// take pointer and shift it to the right until it stops being whitespace
static inline void skipWhitespace(const char **pp) {
    while (isspace(**pp)) (*pp)++;
};

// find matching ']' for '[', and return pointer.
// if any editable fields are NULL, dont change them
// return NULL on malformation
static inline char *findFuncClose(const char *ptr, char **openOut, int *errCode) {
    char *open = strchr((char *)ptr, '[');
    if (!open) {
        if (errCode != NULL) *errCode = E_FUNC_INVALID_BRACKETS;
        return NULL;
    }
    char *close = open;

    // set close, making sure there isnt something bad like [[]
    size_t depth = 0;
    char *validateBracket = close;
    // set final bracket, checking depth and also seeing if its empty
    while (*validateBracket) {
        switch (*validateBracket) {
            case '[': depth++; break;
            case ']': {
                close = validateBracket;
                if (depth > 0) depth--;
                break;
            }
        }

        validateBracket++;
        if (depth == 0) break;
    }
    if (depth || close == open) {
        if (errCode != NULL) *errCode = E_FUNC_INVALID_BRACKETS;
        return NULL;
    }

    if (openOut != NULL) {
        *openOut = open;
    }

    return close;
}

// get '{' and return equivalent '}'
// return NULL on malformation
static inline char *findVarClose(const char *ptr) {
    char *open = strchr((char *)ptr, '{');
    if (!open) {
        return NULL;
    }
    char *close = open;

    // set close, making sure there isnt something bad like {{}
    size_t depth = 0;
    char *validateBracket = close;
    // set final bracket, checking depth and also seeing if its empty
    while (*validateBracket) {
        switch (*validateBracket) {
            case '{': depth++; break;
            case '}': {
                close = validateBracket;
                if (depth > 0) depth--;
                break;
            }
        }

        validateBracket++;
        if (depth == 0) break;
    }
    if (depth || close == open) {
        return NULL;
    }

    return close;
}

// respond to ctrl+c
static inline void handleCtrlC(int sig_num) {
    fprintf(stderr, "\nInterrupted, exiting...\n");
    exit(retCode);
}

// get backup term info and set terminal back to normal
static inline void restoreTerminal(void) {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &backup);
}

// priority list for operators
static inline uint8_t getPriority(const char operation) {
    switch (operation) {
        case '+': case '-': return 1;
        case '*': case '/': case '%': return 2;
        case '^': return 3;
    }
    return 0;
}

// find proper func and return enum equivalent, return -1 on fail
// remember to keep higher strlen ones at the top
static inline int getFuncIndex(const char *ptr) {
    if (!strncmp(ptr, "lgamma", 6)) return L_GAMMA;
    if (!strncmp(ptr, "floor",  5)) return FLOOR;
    if (!strncmp(ptr, "gamma",  5)) return GAMMA;
    if (!strncmp(ptr, "asinh",  5)) return SINE_RH;
    if (!strncmp(ptr, "acosh",  5)) return COSINE_RH;
    if (!strncmp(ptr, "atanh",  5)) return TANGENT_RH;
    if (!strncmp(ptr, "trunc",  5)) return TRUNCATE;
    if (!strncmp(ptr, "log10",  5)) return D_LOG;
    if (!strncmp(ptr, "hypot",  5)) return HYPOTENUSE;
    if (!strncmp(ptr, "log2",   4)) return B_LOG;
    if (!strncmp(ptr, "logx",   4)) return X_LOG;
    if (!strncmp(ptr, "ceil",   4)) return CEILING;
    if (!strncmp(ptr, "sqrt",   4)) return SQUARE_ROOT;
    if (!strncmp(ptr, "cbrt",   4)) return CUBE_ROOT;
    if (!strncmp(ptr, "sinh",   4)) return SINE_H;
    if (!strncmp(ptr, "cosh",   4)) return COSINE_H;
    if (!strncmp(ptr, "tanh",   4)) return TANGENT_H;
    if (!strncmp(ptr, "asin",   4)) return SINE_R;
    if (!strncmp(ptr, "acos",   4)) return COSINE_R;
    if (!strncmp(ptr, "atan",   4)) return TANGENT_R;
    if (!strncmp(ptr, "erfc",   4)) return ERROR_FUNC_C;
    if (!strncmp(ptr, "erf",    3)) return ERROR_FUNC;
    if (!strncmp(ptr, "sin",    3)) return SINE;
    if (!strncmp(ptr, "cos",    3)) return COSINE;
    if (!strncmp(ptr, "tan",    3)) return TANGENT;
    if (!strncmp(ptr, "log",    3)) return N_LOG;
    return -1;
}

// get validateBuffer error and spit out string
static inline char *retToStr(char err) {
    switch (err) {
        case E_INVALID_CHAR:             return (char *)"Invalid character in formula."; break;
        case E_INSUFFICIENT_CLOSE_PAREN: return (char *)"Not enough closing parentheses."; break;
        case E_INSUFFICIENT_OPEN_PAREN:  return (char *)"Not enough opening parentheses."; break;
        case E_INVALID_OPERAND:          return (char *)"Invalid operand."; break;
        case E_INVALID_OPERATOR:         return (char *)"Invalid operator."; break;
        case E_INSUFFICIENT_NUMS:        return (char *)"Not enough numbers for calculation"; break;
        case E_INSUFFICIENT_OPS:         return (char *)"Not enough operators for calculation"; break;
        case E_EMPTY_PAREN:              return (char *)"Empty parentheses."; break;
        case E_FUNC_INVALID_BRACKETS:    return (char *)"A function has invalid brackets."; break;
        case E_EMPTY_FUNCTION:           return (char *)"A function has no contents."; break;
        case E_VAR_OPEN_BRACKETS:        return (char *)"Invalid variable insertion, unclosed curly brackets."; break;
        case E_VAR_UNKNOWN:              return (char *)"Invalid variable insertion, unknown variable."; break;
        case E_MULTI_ARG_INSUFFICIENT:   return (char *)"Not enough arguments for a multi-arg function."; break;
        case E_MULTI_ARG_EXCESS:         return (char *)"Too many arguments for a multi-arg function."; break;
        case E_MULTI_ARG_INVALID_FIRST:  return (char *)"Invalid first argument for a multi-arg function."; break;
        case E_MULTI_ARG_INVALID_SECOND: return (char *)"Invalid second argument for a multi-arg function."; break;
        case E_DISPLAY_SIZE_LIMIT:       return (char *)"Result display size limit reached - Sorry!"; break;
        case E_INPUT_SIZE_LIMIT:         return (char *)"Input size limit reached - Sorry!"; break;
        default: return (char *)"Unknown error num - Sorry! :p"; break;
    }
}

// calculate 3 tokens
static inline defaultPrecision calculateTrio(defaultPrecision left, char op, defaultPrecision right) {
    defaultPrecision result = 0;
    switch (op) {
        case '+': result = left + right; break;
        case '-': result = left - right; break;
        case '/': result = left / right; break;
        case '*': result = left * right; break;
        case '^': result = pow(left, right); break;
        case '%': result = fmod(left, right); break;
        default: break;
    }

    return result;
}

// for double argument functions, find the ',' for the main function
// return NULL on malformation
// assumes that buf starts right after function opener ('[')
static inline char *findFuncComma(const char *buf, const char *end) {
    char *ptr = (char *)buf;
    unsigned int depth = 0;

    while (ptr != end) {
        // tweak depth
        if (*ptr == '(' || *ptr == '[' || *ptr == '{') depth++;
        if (*ptr == ')' || *ptr == ']' || *ptr == '}') depth--;

        // set comma
        if (*ptr == ',' && depth == 0) return ptr;

        ptr++;
    }

    return NULL;
}

#endif
