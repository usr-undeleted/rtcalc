#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <stdint.h>
// use more precision or not
#ifdef USE_LONG_DOUBLE
#define FORMAT_WITH_PRECISION "%.*Lf"
typedef long double defaultPrecision;
// functions (df for defined function)
#define DF_SQRT(x)       sqrtl  (x)
#define DF_CBRT(x)       cbrtl  (x)
#define DF_SIN(x)        sinl   (x)
#define DF_COS(x)        cosl   (x)
#define DF_TAN(x)        tanl   (x)
#define DF_SINH(x)       sinhl  (x)
#define DF_COSH(x)       coshl  (x)
#define DF_TANH(x)       tanhl  (x)
#define DF_ASIN(x)       asinl  (x)
#define DF_ACOS(x)       acosl  (x)
#define DF_ATAN(x)       atanl  (x)
#define DF_ASINH(x)      asinhl (x)
#define DF_ACOSH(x)      acoshl (x)
#define DF_ATANH(x)      atanhl (x)
#define DF_CSC(x)        1.0 / DF_SIN (x)
#define DF_SEC(x)        1.0 / DF_COS (x)
#define DF_COT(x)        1.0 / DF_TAN (x)
#define DF_CSCH(x)       1.0 / DF_SINH(x)
#define DF_SECH(x)       1.0 / DF_COSH(x)
#define DF_COTH(x)       1.0 / DF_TANH(x)
#define DF_FLOOR(x)      floorl (x)
#define DF_CEIL(x)       ceill  (x)
#define DF_LOG(x)        logl   (x)
#define DF_LOG10(x)      log10l (x)
#define DF_LOG2(x)       log2l  (x)
#define DF_GAMMA(x)      gammal (x)
#define DF_TRUNC(x)      truncl (x)
#define DF_ERF(x)        erfl   (x)
#define DF_ERFC(x)       erfcl  (x)
#define DF_LGAMMA(x)     lgammal(x)
#define DF_ABS(x)        fabsl  (x)
#define DF_ROUND(x)      roundl (x)
#define DF_DEG(x)        x / M_PI * 180.0
#define DF_RAD(x)        x / 180.0 * M_PI
#define DF_EXP(x)        expl   (x)
#define DF_EXP2(x)       exp2l  (x)
// multi args below
#define DF_FMOD(x, y)    fmodl (x ,y)
#define DF_FMAX(x, y)    fmaxl (x, y)
#define DF_FMIN(x, y)    fminl (x, y)
#define DF_POW(x,y)      powl  (x,y)
#define DF_HYPOT(x,y)    hypotl(x,y)
#define DF_LOGX(x,y)     DF_LOG(y) / DF_LOG(x)
#define DF_ATAN2(x,y)    atan2l(x,y)
#else
#define FORMAT_WITH_PRECISION "%.*lf"
typedef double defaultPrecision;
// functions (df for defined function)
#define DF_SQRT(x)       sqrt  (x)
#define DF_CBRT(x)       cbrt  (x)
#define DF_SIN(x)        sin   (x)
#define DF_COS(x)        cos   (x)
#define DF_TAN(x)        tan   (x)
#define DF_SINH(x)       sinh  (x)
#define DF_COSH(x)       cosh  (x)
#define DF_TANH(x)       tanh  (x)
#define DF_ASIN(x)       asin  (x)
#define DF_ACOS(x)       acos  (x)
#define DF_ATAN(x)       atan  (x)
#define DF_ASINH(x)      asinh (x)
#define DF_ACOSH(x)      acosh (x)
#define DF_ATANH(x)      atanh (x)
#define DF_CSC(x)        1.0 / DF_SIN (x)
#define DF_SEC(x)        1.0 / DF_COS (x)
#define DF_COT(x)        1.0 / DF_TAN (x)
#define DF_CSCH(x)       1.0 / DF_SINH(x)
#define DF_SECH(x)       1.0 / DF_COSH(x)
#define DF_COTH(x)       1.0 / DF_TANH(x)
#define DF_FLOOR(x)      floor (x)
#define DF_CEIL(x)       ceil  (x)
#define DF_LOG(x)        log   (x)
#define DF_LOG10(x)      log10 (x)
#define DF_LOG2(x)       log2  (x)
#define DF_GAMMA(x)      gamma (x)
#define DF_TRUNC(x)      trunc (x)
#define DF_ERF(x)        erf   (x)
#define DF_ERFC(x)       erfc  (x)
#define DF_LGAMMA(x)     lgamma(x)
#define DF_ABS(x)        fabs  (x)
#define DF_ROUND(x)      round (x)
#define DF_DEG(x)        x / M_PI * 180.0
#define DF_RAD(x)        x / 180.0 * M_PI
#define DF_EXP(x)        exp   (x)
#define DF_EXP2(x)       exp2  (x)
// multi args below
#define DF_FMOD(x, y)    fmod (x ,y)
#define DF_FMAX(x, y)    fmax (x, y)
#define DF_FMIN(x, y)    fmin (x, y)
#define DF_POW(x,y)      pow  (x, y)
#define DF_HYPOT(x,y)    hypot(x, y)
#define DF_ATAN2(x,y)    atan2(x, y)
#define DF_LOGX(x,y)     DF_LOG(y) / DF_LOG(x)
#endif // USE_LONG_DOUBLE
// custom funcs
#define DF_CUSTOM_RNDFLR(x, y) roundfloor(x, y);
#define DF_CUSTOM_RNDCIL(x, y) roundceil (x, y);

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
#define VERSION "release 1.28.8"
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
    COSECANT,
    SECANT,
    COTANGENT,
    COSECANT_H,
    SECANT_H,
    COTANGENT_H,
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
    ROUND,
    RAD_TO_DEG,
    DEG_TO_RAD,
    EXPONENT_E,
    EXPONENT_2,
    // the enum below is padding for multi arg funcs
    // done in order to make our lifes easier
    // multi arg funcs HAVE to be below this padding
    // single arg funcs HAVE to be above this padding
    MULTI_ARG_FUNC_PADDING,
    // now, everything down here is multi-arg
    POWER_FUNC,
    MAXIMUM,
    MINIMUN,
    HYPOTENUSE,
    X_LOG,
    TANGENT_A2,
    // custom multi-args
    CUSTOM_ROUND_FLOOR,
    CUSTOM_ROUND_CEILING,
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
