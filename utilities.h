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
        "\n"

        "\e[3mUsage:\e[0m\n"
        "- This tool, of course, follows basic math principles.\n"
        "\e[1mBasic examples:\e[0m\n"
        "\e[4m10 + 2 * 3\e[0m\n"
        "\e[4m2 ^ (10 * (40 / 2))\e[0m\n\n"

        "- Basic operators are:\n"
        "\e[1mAddition\e[0m (+)\n"
        "\e[1mSubtraction\e[0m (-)\n"
        "\e[1mMultiplication\e[0m (*)\n"
        "\e[1mDivision\e[0m (/)\n"
        "\e[1mExponents\e[0m (^)\n"
        "\e[1mRemainder of division\e[0m (%%)\n"
        "\n"

        "- This tool also presents math functions, those being:\n"
        "\e[1mSquare root:\e[0m\n"
        "- \e[4msqrt[x]\e[0m, results in the square root of \e[1m'x'.\e[0m\n"

        "\e[1mCube root:\e[0m\n"
        "- \e[4mcbrt[x]\e[0m, results in the cube root of \e[1m'x'.\e[0m\n"

        "\e[1mSine:\e[0m\n"
        "- \e[4msin[x]\e[0m, results in the sine of \e[1m'x'.\e[0m\n"

        "\e[1mCosine:\e[0m\n"
        "- \e[4mcos[x]\e[0m, results in the cosine of \e[1m'x'.\e[0m\n"

        "\e[1mTangent:\e[0m\n"
        "- \e[4mtan[x]\e[0m, results in the tangent of \e[1m'x'.\e[0m\n"

        "\e[1mNatural logarithm:\e[0m\n"
        "- \e[4mlog[x]\e[0m, results in the natural logarithm of \e[1m'x'.\e[0m\n"

        "\e[1mHyperbolic sine:\e[0m\n"
        "- \e[4msinh[x]\e[0m, results in the hyperbolic sine of \e[1m'x'.\e[0m\n"

        "\e[1mHyperbolic cosine:\e[0m\n"
        "- \e[4mcohs[x]\e[0m, results in the hyperbolic cosine of \e[1m'x'.\e[0m\n"

        "\e[1mHyperbolic tangent:\e[0m\n"
        "- \e[4mtanh[x]\e[0m, results in the hyperbolic tangent of \e[1m'x'.\e[0m\n"

        "\e[1mFloor:\e[0m\n"
        "- \e[4msin[x]\e[0m, results in the floor of \e[1m'x'.\e[0m\n"

        "\e[1mCeiling:\e[0m\n"
        "- \e[4mceil[x]\e[0m, results in the ceiling of \e[1m'x'.\e[0m\n"
        "\n" // end of function list

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
// if either childBuf or childLine are NULL, skip their definitions.
// return NULL on malformation
static inline char *findFuncClose(const char *ptr, char **openOut, int *errCode) {
    char *open = strchr((char *)ptr, '[');
    if (!open) {
        if (errCode != NULL) *errCode = 10;
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
        if (errCode != NULL) *errCode = 10;
        return NULL;
    }

    if (openOut != NULL) {
        *openOut = open;
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
static inline int getFuncIndex(const char *ptr) {
    if (!strncmp(ptr, "sqrt", 4))  return SQUARE_ROOT;
    if (!strncmp(ptr, "cbrt", 4))  return CUBE_ROOT;
    if (!strncmp(ptr, "sin",  3))  return SINE;
    if (!strncmp(ptr, "cos",  3))  return COSINE;
    if (!strncmp(ptr, "tan",  3))  return TANGENT;
    if (!strncmp(ptr, "log",  3))  return N_LOG;
    if (!strncmp(ptr, "sinh", 4))  return SINE_H;
    if (!strncmp(ptr, "cosh", 4))  return COSINE_H;
    if (!strncmp(ptr, "tanh", 4))  return TANGENT_H;
    if (!strncmp(ptr, "floor", 5)) return FLOOR;
    if (!strncmp(ptr, "ceil",  4)) return CEILING;
    return -1;
}

// used by func above
static inline char *retToStr(char err) {
    switch (err) {
        case  1: return (char *)"Invalid character in formula."; break;
        case  2: return (char *)"Not enough closing parentheses."; break;
        case  3: return (char *)"Not enough opening parentheses."; break;
        case  4: return (char *)"Invalid operand."; break;
        case  5: return (char *)"Invalid operator."; break;
        case  6: return (char *)"Not enough numbers for calculation"; break;
        case  7: return (char *)"Not enough operators for calculation"; break;
        case  8: return (char *)"Empty parentheses."; break;
        case  9: return (char *)"Input size limit reached - Sorry!"; break;
        case 10: return (char *)"A function has invalid brackets."; break;
        case 11: return (char *)"A function has no contents."; break;
        case 12: return (char *)"Result display size limit reached - Sorry!"; break;
        default: return (char *)"Unknown error num - Sorry! :p"; break;
    }
}

// calculate 3 tokens
static inline double calculateTrio(double left, char op, double right) {
    double result = 0;
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

#endif
