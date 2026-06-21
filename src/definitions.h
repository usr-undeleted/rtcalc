#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <stdint.h>
// use more precision or not
#ifdef USE_LONG_DOUBLE
#define FORMAT_WITH_PRECISION "%.*Lf"
typedef long double defaultPrecision;
#else
#define FORMAT_WITH_PRECISION "%.*lf"
typedef double defaultPrecision;
#endif

enum tokenType {
    SKIP,
    NUMBER,
    OPERATOR,
    PARENTHESES
};

enum tokenColorType {
    SC_EMPTY,
    SC_NUMBER,
    SC_OPERATOR,
    SC_FUNCTION,
    SC_PARENTHESES,
    SC_BRACKETS,
    SC_CURLY_BRACKETS,
    SC_VARIABLES,
    SC_COMMA
};

// used by calculateBuffer
struct calcToken {
    // the kind of token to be worked with, defined by
    // tokenType enum
    enum tokenType type;
    // value of the token
    defaultPrecision val;
    // operator of the token
    char op;
    // how deep a '(' is
    unsigned int depth;
    // where a token is located in it's original string
    char *ptr;
};

// used by syntax coloring
struct colorToken {
    // the kind of token, to determine its color
    enum tokenColorType type;
    // where a token is located in it's original string
    char *ptr;
};

#define BUFFER_SIZE 2049UL // for both result and buf size, actual limit is - 1
#define RESULT_SIZE 1025UL
#define USER_MISTAKE 2
#define CODE_MISTAKE 1
#define WELCOME "Welcome to the realtime CLI math tool!"
#define VALID_LIST "0123456789+-*/().^% "
#define OPERATIONS "+-*/^%"
#define DELIMITERS ".()[]+-*/^%{},"
// x.y.z
// x for big, monumental changes, or milestones
// y for addition of new features
// z for fixes and small changes
#define VERSION "release 1.24.7"
// count token flags
#define CT_FLAG_EMPTY 0
#define CT_FLAG_READ_BRACKETS 1
#define CT_FLAG_READ_CURLY_BRACKETS 2
#define CT_FLAG_READ_COMMAS 4

// colors
#define RESET     "\e[0m"
#define BOLD      "\e[1m"
#define DIM       "\e[2m"
#define ITALIC    "\e[3m"
#define UNDERLINE "\e[4m"
#define RED       "\e[31m"
#define GREEN     "\e[32m"
#define YELLOW    "\e[33m"
#define BLUE      "\e[34m"
#define MAGENTA   "\e[35m"
#define CYAN      "\e[36m"
#define BLACK     "\e[37m"

// error
#define ERROR_CLR RED
// syntax highlighting
#define NUMBER_CLR         YELLOW
#define OPERATOR_CLR       MAGENTA
#define FUNCTION_CLR       BLUE
#define BRACKETS_CLR       GREEN
#define PAREN_CLR          BLUE
#define CURLY_BRACKETS_CLR RED
#define VARIABLE_CLR       YELLOW
#define COMMA_CLR          GREEN

// flags for main() to use
extern unsigned char globalFlags;
// syntax highlight
#define USE_PRETTY_COLORS 1

// what function was found by func indexer
enum funcIndex {
    SQUARE_ROOT,
    CUBE_ROOT,
    SINE,
    COSINE,
    TANGENT,
    SINE_H,
    COSINE_H,
    TANGENT_H,
    SINE_R,
    COSINE_R,
    TANGENT_R,
    SINE_RH,
    COSINE_RH,
    TANGENT_RH,
    FLOOR,
    CEILING,
    N_LOG,
    D_LOG,
    B_LOG,
    GAMMA,
    TRUNCATE,
    ERROR_FUNC,
    ERROR_FUNC_C,
    L_GAMMA,
    ABSOLUTE,
    // the enum below is padding for multi arg funcs
    // done in order to make our lifes easier
    // multi arg funcs HAVE to be below this padding
    // single arg funcs HAVE to be above this padding
    MULTI_ARG_FUNC_PADDING,
    // now, everything down here is multi-arg
    MAXIMUM,
    MINIMUN,
    HYPOTENUSE,
    X_LOG,
    TANGENT_A2,
};

// used by retToStr()
enum retErrStr {
    // ret is checked if non zero to error, therefore, padding on 0
    E_SEEMINGLY_OKAY,
    // everything down is a real error
    E_INVALID_CHAR,
    E_INSUFFICIENT_CLOSE_PAREN,
    E_INSUFFICIENT_OPEN_PAREN,
    E_EMPTY_PAREN,
    E_INVALID_OPERAND,
    E_INVALID_OPERATOR,
    E_INSUFFICIENT_NUMS,
    E_INSUFFICIENT_OPS,
    E_FUNC_INVALID_BRACKETS,
    E_EMPTY_FUNCTION,
    E_VAR_OPEN_BRACKETS,
    E_VAR_UNKNOWN,
    E_MULTI_ARG_INSUFFICIENT,
    E_MULTI_ARG_EXCESS,
    E_MULTI_ARG_INVALID_FIRST,
    E_MULTI_ARG_INVALID_SECOND,
    E_DISPLAY_SIZE_LIMIT,
    E_INPUT_SIZE_LIMIT,
};

// environment variable
#define MAX_VARIABLES 32
struct variable {
    defaultPrecision value;
    uint64_t len;
    char *name;
};

#endif
