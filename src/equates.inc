; equates.inc - All numeric equates for rtcalc
; Included by every .asm file via %include "equates.inc"

; Sizes
%define BUFFER_SIZE       2049
%define RESULT_SIZE       1025
%define MAX_VARIABLES     32

; Exit codes
%define USER_MISTAKE      2
%define CODE_MISTAKE      1

; Global flags
%define USE_PRETTY_COLORS  1

; tokenType enum
%define TOK_SKIP          0
%define TOK_NUMBER        1
%define TOK_OPERATOR      2
%define TOK_PARENTHESES   3

; tokenColorType enum
%define SC_EMPTY          0
%define SC_NUMBER         1
%define SC_OPERATOR       2
%define SC_FUNCTION       3
%define SC_PARENTHESES    4
%define SC_BRACKETS       5
%define SC_CURLY_BRACKETS 6
%define SC_VARIABLES      7
%define SC_COMMA          8

; countTokens flags
%define CT_FLAG_EMPTY             0
%define CT_FLAG_READ_BRACKETS     1
%define CT_FLAG_READ_CURLY_BRACKETS 2
%define CT_FLAG_READ_COMMAS       4

; funcIndex enum
%define FN_SQUARE_ROOT    0
%define FN_CUBE_ROOT      1
%define FN_SINE           2
%define FN_COSINE         3
%define FN_TANGENT        4
%define FN_SINE_H         5
%define FN_COSINE_H       6
%define FN_TANGENT_H      7
%define FN_SINE_R         8
%define FN_COSINE_R       9
%define FN_TANGENT_R      10
%define FN_SINE_RH        11
%define FN_COSINE_RH      12
%define FN_TANGENT_RH     13
%define FN_COSECANT       14
%define FN_SECANT         15
%define FN_COTANGENT      16
%define FN_COSECANT_H     17
%define FN_SECANT_H       18
%define FN_COTANGENT_H    19
%define FN_FLOOR          20
%define FN_CEILING        21
%define FN_N_LOG          22
%define FN_D_LOG          23
%define FN_B_LOG          24
%define FN_GAMMA          25
%define FN_TRUNCATE       26
%define FN_ERROR_FUNC     27
%define FN_ERROR_FUNC_C   28
%define FN_L_GAMMA        29
%define FN_ABSOLUTE       30
%define FN_ROUND          31
%define FN_RAD_TO_DEG     32
%define FN_DEG_TO_RAD     33
%define FN_EXPONENT_E     34
%define FN_EXPONENT_2     35
%define FN_MULTI_ARG_PAD  36
%define FN_POWER_FUNC     37
%define FN_MAXIMUM        38
%define FN_MINIMUN        39
%define FN_HYPOTENUSE     40
%define FN_X_LOG          41
%define FN_TANGENT_A2     42

; retErrStr enum
%define E_SEEMINGLY_OKAY           0
%define E_INVALID_CHAR             1
%define E_INSUFFICIENT_CLOSE_PAREN 2
%define E_INSUFFICIENT_OPEN_PAREN  3
%define E_EMPTY_PAREN              4
%define E_INVALID_OPERAND          5
%define E_INVALID_OPERATOR         6
%define E_INSUFFICIENT_NUMS        7
%define E_INSUFFICIENT_OPS         8
%define E_FUNC_INVALID_BRACKETS    9
%define E_EMPTY_FUNCTION           10
%define E_VAR_OPEN_BRACKETS        11
%define E_VAR_UNKNOWN              12
%define E_MULTI_ARG_INSUFFICIENT   13
%define E_MULTI_ARG_EXCESS         14
%define E_MULTI_ARG_INVALID_FIRST  15
%define E_MULTI_ARG_INVALID_SECOND 16
%define E_DISPLAY_SIZE_LIMIT       17
%define E_INPUT_SIZE_LIMIT         18

; struct calcToken layout (32 bytes)
%define TOK_OFF_TYPE   0
%define TOK_OFF_VAL    8
%define TOK_OFF_OP     16
%define TOK_OFF_DEPTH  20
%define TOK_OFF_PTR    24
%define TOK_SIZE       32

; struct colorToken layout (16 bytes)
%define CTOK_OFF_TYPE  0
%define CTOK_OFF_PTR   8
%define CTOK_SIZE      16

; struct variable layout (24 bytes)
%define VAR_OFF_VALUE  0
%define VAR_OFF_LEN    8
%define VAR_OFF_NAME   16
%define VAR_SIZE       24

; termios offsets (Linux x86-64)
%define TERMIOS_IFLAG  0
%define TERMIOS_OFLAG  4
%define TERMIOS_CFLAG  8
%define TERMIOS_LFLAG  12
%define TERMIOS_LINE   16
%define TERMIOS_CC     17
%define TERMIOS_ISPEED 32
%define TERMIOS_OSPEED 36

; termios flags
%define ICANON          0o0000002
%define ECHO_FLAG       0o0000010
%define VTIME_INDEX     5
%define VMIN_INDEX      6

; common ASCII
%define ESC_CHAR        0x1B
%define DEL_CHAR        127
%define BS_CHAR         0x08
%define CTRL_W          0x17
%define CTRL_X          0x18
%define CTRL_K          0x0B
%define CTRL_A          0x01
%define CTRL_E          0x05
%define NL_CHAR         0x0A
%define FF_CHAR         0x0C
