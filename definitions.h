#ifndef DEFINITIONS_H
#define DEFINITIONS_H

enum tokenType {
    SKIP,
    NUMBER,
    OPERATOR,
    PARENTHESES
};

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
#define VERSION "release 1.7.2"

enum funcIndex {
    SQUARE_ROOT,
    CUBE_ROOT,
    SINE,
    COSINE,
    TANGENT,
    N_LOG
};

#endif
