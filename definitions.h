#ifndef DEFINITIONS_H
#define DEFINITIONS_H

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
};

// used by calculateBuffer
struct calcToken {
    // the kind of token to be worked with, defined by
    // tokenType enum
    enum tokenType type;
    // value of the token
    double val;
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

#define BUFFER_SIZE 4097UL // 4096 is the actual limit
#define RESULT_SIZE 2049UL
#define USER_MISTAKE 2
#define CODE_MISTAKE 1
#define WELCOME "Welcome to rtcalc!\n"
#define VALID_LIST "0123456789+-*/().^ "
#define OPERATIONS "+-*/^"
// x.y.z
// x for big, monumental changes
// y for addition of new features
// z for fixes
#define VERSION "release 1.9.3"
#define CT_FLAG_EMPTY 0
#define CT_FLAG_READ_BRACKETS 1

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
#define NUMBER_CLR   YELLOW
#define OPERATOR_CLR MAGENTA
#define FUNCTION_CLR BLUE
#define BRACKETS_CLR GREEN
#define PAREN_CLR    BLUE

// flags for main() to use
extern unsigned char globalFlags;
// syntax highlight
#define USE_PRETTY_COLORS 1

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
};

#endif
