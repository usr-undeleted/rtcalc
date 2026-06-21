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
    SC_VARIABLES
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

#define BUFFER_SIZE 2049UL // 4096 is the actual limit
#define RESULT_SIZE 1025UL
#define USER_MISTAKE 2
#define CODE_MISTAKE 1
#define WELCOME "Welcome to the realtime CLI math tool!"
#define VALID_LIST "0123456789+-*/().^% "
#define OPERATIONS "+-*/^%"
#define DELIMITERS ".()[]+-*/^%{}"
// x.y.z
// x for big, monumental changes, or milestones
// y for addition of new features
// z for fixes and small changes
#define VERSION "release 1.15.4"
// count token flags
#define CT_FLAG_EMPTY 0
#define CT_FLAG_READ_BRACKETS 1
#define CT_FLAG_READ_CURLY_BRACKETS 2

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

// flags for main() to use
extern unsigned char globalFlags;
// syntax highlight
#define USE_PRETTY_COLORS 1
#define DEFINE_ENV_VARS 2

enum funcIndex {
    SQUARE_ROOT,
    CUBE_ROOT,
    SINE,
    COSINE,
    TANGENT,
    N_LOG,
    SINE_H,
    COSINE_H,
    TANGENT_H,
    FLOOR,
    CEILING,
    GAMMA,
};

// environment variable
#define MAX_VARIABLES 32
struct variable {
    defaultPrecision value;
    uint64_t len;
    char *name;
};

#endif
