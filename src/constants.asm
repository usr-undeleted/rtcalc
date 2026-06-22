; constants.asm - All data and BSS for rtcalc
; Equates are in equates.inc (included by all .asm files)

%include "equates.inc"

SECTION .rodata

; Declare all exported symbols as global
global str_welcome, str_valid_list, str_operations, str_delimiters
global str_version, str_default_prompt, str_format_double
global str_reset, str_clr_error, str_clr_number, str_clr_operator
global str_clr_function, str_clr_brackets, str_clr_paren, str_clr_curly
global str_clr_variable, str_clr_comma
global str_esc_save, str_esc_restore, str_esc_repl, str_esc_cr_down
global str_help_header, str_help_legal, str_help_usage
global str_help_details, str_help_args
global err_unknown, err_invalid_char, err_insuf_close_paren, err_insuf_open_paren
global err_invalid_operand, err_invalid_operator, err_insuf_nums, err_insuf_ops
global err_empty_paren, err_func_invalid_brackets, err_empty_function
global err_var_open_brackets, err_var_unknown
global err_multi_arg_insuf, err_multi_arg_excess
global err_multi_arg_invalid_first, err_multi_arg_invalid_second
global err_display_size_limit, err_input_size_limit
global str_err_prefix1, str_err_prefix2, str_err_suffix, str_cant_calc
global str_arg_help, str_arg_prompt, str_arg_syntax, str_arg_precision, str_arg_define
global func_table
global dpi, d180, d1
global g_backup, g_retCode, g_globalFlags, g_precision, g_prompt
global g_varCount, g_variables

; ---- Strings ----
str_welcome:        db  "Welcome to the realtime CLI math tool!", 0
str_valid_list:     db  "0123456789+-*/().^% ", 0
str_operations:     db  "+-*/^%", 0
str_delimiters:     db  ".()[]+-*/^{},", 0
str_version:        db  "release 1.28.8", 0
str_default_prompt: db  ">>> ", 0
str_format_double:  db  "%.*lf", 0
str_format_long:    db  "%.*Lf", 0

; ANSI escape codes
str_reset:          db  27, "[0m", 0
str_bold:           db  27, "[1m", 0
str_dim:            db  27, "[2m", 0
str_italic:         db  27, "[3m", 0
str_underline:      db  27, "[4m", 0
str_red:            db  27, "[31m", 0
str_green:          db  27, "[32m", 0
str_yellow:         db  27, "[33m", 0
str_blue:           db  27, "[34m", 0
str_magenta:        db  27, "[35m", 0
str_cyan:           db  27, "[36m", 0
str_black:          db  27, "[37m", 0

; Color aliases for syntax highlighting
str_clr_error:      dq str_red
str_clr_number:     dq str_yellow
str_clr_operator:   dq str_magenta
str_clr_function:   dq str_blue
str_clr_brackets:   dq str_green
str_clr_paren:      dq str_blue
str_clr_curly:      dq str_red
str_clr_variable:   dq str_yellow
str_clr_comma:      dq str_green
str_esc_save:       db  27, "[s", 0          ; save cursor
str_esc_restore:    db  27, "[u", 27, "[J", 0 ; restore + clear to end
str_esc_repl:       db  27, "[A", 13, 27, "[2K", 0  ; up, cr, clear line
str_esc_cr_down:    db  13, 27, "[B", 0      ; cr, down

; ---- Help menu (formatted with printf) ----
str_help_header:    db  27, "[1mReal-time calculation tool (rtcalc)", 27, "[0m", 10, 0
str_help_legal:     db  27, "[3mLegal and basic info:", 27, "[0m", 10, \
                       "- Licensed under the GNU GPL-3.0 license. Open-source, and free, forever.", 10, \
                       "- Source code hosted under Github (https://github.com/usr-undeleted/rtcalc).", 10, \
                       "- Current version: ", 27, "[1m", "%s", 27, "[0m", 10, \
                       "- Made with love, by Undeleted. <3", 10, \
                       "- Functions, operators and basic syntax can be found in this program's manual.", 10, \
                       10, 0
str_help_usage:     db  27, "[3mUsage:", 27, "[0m", 10, \
                       "- This tool, of course, follows basic math principles.", 10, \
                       27, "[1mBasic examples:", 27, "[0m", 10, \
                       27, "[4m10 + 2 * 3", 27, "[0m", 10, \
                       27, "[4m2 ^ (10 * (40 / 2))", 27, "[0m", 10, 10, 0
str_help_details:   db  27, "[3mDetails:", 27, "[0m", 10, \
                       "- Invalid input will lead to an error, preventing calculation.", 10, \
                       "- Input is limited to %zu characters.", 10, \
                       "- Result size is limited to %zu characters.", 10, 10, 0
str_help_args:      db  27, "[3mAdditional arguments:", 27, "[0m", 10, \
                       "- ", 27, "[1m", '"help"', 27, "[0m", ": Show this menu.", 10, \
                       "- ", 27, "[1m", '"prompt=<prompt>"', 27, "[0m", \
                       ": Define a custom prompt before startup (space not included).", 10, \
                       "- ", 27, "[1m", '"syntax-highlighting"', 27, "[0m", \
                       ": Enable syntax highlighting on the input.", 10, \
                       "- ", 27, "[1m", '"precision=<num>"', 27, "[0m", \
                       ": Define what precision to show results in. Defaults to 6.", 10, \
                       "- ", 27, "[1m", '"define:<name>=<num>"', 27, "[0m", \
                       ": Create an immutable variable accessible during runtime.", 10, \
                       "%s", 0

; ---- Error messages ----
err_invalid_char:            db "Invalid character in formula.", 0
err_insuf_close_paren:       db "Not enough closing parentheses.", 0
err_insuf_open_paren:        db "Not enough opening parentheses.", 0
err_invalid_operand:         db "Invalid operand.", 0
err_invalid_operator:        db "Invalid operator.", 0
err_insuf_nums:              db "Not enough numbers for calculation", 0
err_insuf_ops:               db "Not enough operators for calculation", 0
err_empty_paren:             db "Empty parentheses.", 0
err_func_invalid_brackets:   db "A function has invalid brackets.", 0
err_empty_function:          db "A function has no contents.", 0
err_var_open_brackets:       db "Invalid variable insertion, unclosed curly brackets.", 0
err_var_unknown:             db "Invalid variable insertion, unknown variable.", 0
err_multi_arg_insuf:         db "Not enough arguments for a multi-arg function.", 0
err_multi_arg_excess:        db "Too many arguments for a multi-arg function.", 0
err_multi_arg_invalid_first: db "Invalid first argument for a multi-arg function.", 0
err_multi_arg_invalid_second: db "Invalid second argument for a multi-arg function.", 0
err_display_size_limit:      db "Result display size limit reached - Sorry!", 0
err_input_size_limit:        db "Input size limit reached - Sorry!", 0
err_unknown:                 db "Unknown error num - Sorry! :p", 0

; Prefixed error strings (used in main loop)
str_err_prefix1:    db 10, 27, "[31m", 0
str_err_prefix2:    db 10, 27, "[31mError: ", 0
str_err_suffix:     db 27, "[0m", 10, 0
str_cant_calc:      db "Can't calculate: %s", 0

; Argument parsing strings
str_arg_help:       db "help", 0
str_arg_prompt:     db "prompt", 6
str_arg_syntax:     db "syntax-highlighting", 0
str_arg_precision:  db "precision", 9
str_arg_define:     db "define", 5

; Function name strings for getFuncIndex (null-terminated)
func_lgamma:        db "lgamma", 0
func_atan2:         db "atan2", 0
func_floor:         db "floor", 0
func_gamma:         db "gamma", 0
func_asinh:         db "asinh", 0
func_acosh:         db "acosh", 0
func_atanh:         db "atanh", 0
func_trunc:         db "trunc", 0
func_log10:         db "log10", 0
func_hypot:         db "hypot", 0
func_round:         db "round", 0
func_log2:          db "log2", 0
func_logx:          db "logx", 0
func_ceil:          db "ceil", 0
func_sqrt:          db "sqrt", 0
func_cbrt:          db "cbrt", 0
func_sinh:          db "sinh", 0
func_cosh:          db "cosh", 0
func_tanh:          db "tanh", 0
func_asin:          db "asin", 0
func_acos:          db "acos", 0
func_atan:          db "atan", 0
func_erfc:          db "erfc", 0
func_fmax:          db "fmax", 0
func_fmin:          db "fmin", 0
func_csch:          db "csch", 0
func_sech:          db "sech", 0
func_coth:          db "coth", 0
func_exp2:          db "exp2", 0
func_pow:           db "pow", 0
func_erf:           db "erf", 0
func_sin:           db "sin", 0
func_cos:           db "cos", 0
func_tan:           db "tan", 0
func_log:           db "log", 0
func_abs:           db "abs", 0
func_deg:           db "deg", 0
func_rad:           db "rad", 0
func_exp:           db "exp", 0
func_csc:           db "csc", 0
func_sec:           db "sec", 0
func_cot:           db "cot", 0

; Function lookup table: {name_ptr, name_len, enum_val}
; Order matters: longer names first (matches C code)
align 8
func_table:
    dq func_lgamma, 6, FN_L_GAMMA
    dq func_atan2,  5, FN_TANGENT_A2
    dq func_floor,  5, FN_FLOOR
    dq func_gamma,  5, FN_GAMMA
    dq func_asinh,  5, FN_SINE_RH
    dq func_acosh,  5, FN_COSINE_RH
    dq func_atanh,  5, FN_TANGENT_RH
    dq func_trunc,  5, FN_TRUNCATE
    dq func_log10,  5, FN_D_LOG
    dq func_hypot,  5, FN_HYPOTENUSE
    dq func_round,  5, FN_ROUND
    dq func_log2,   4, FN_B_LOG
    dq func_logx,   4, FN_X_LOG
    dq func_ceil,   4, FN_CEILING
    dq func_sqrt,   4, FN_SQUARE_ROOT
    dq func_cbrt,   4, FN_CUBE_ROOT
    dq func_sinh,   4, FN_SINE_H
    dq func_cosh,   4, FN_COSINE_H
    dq func_tanh,   4, FN_TANGENT_H
    dq func_asin,   4, FN_SINE_R
    dq func_acos,   4, FN_COSINE_R
    dq func_atan,   4, FN_TANGENT_R
    dq func_erfc,   4, FN_ERROR_FUNC_C
    dq func_fmax,   4, FN_MAXIMUM
    dq func_fmin,   4, FN_MINIMUN
    dq func_csch,   4, FN_COSECANT_H
    dq func_sech,   4, FN_SECANT_H
    dq func_coth,   4, FN_COTANGENT_H
    dq func_exp2,   4, FN_EXPONENT_2
    dq func_pow,    3, FN_POWER_FUNC
    dq func_erf,    3, FN_ERROR_FUNC
    dq func_sin,    3, FN_SINE
    dq func_cos,    3, FN_COSINE
    dq func_tan,    3, FN_TANGENT
    dq func_log,    3, FN_N_LOG
    dq func_abs,    3, FN_ABSOLUTE
    dq func_deg,    3, FN_RAD_TO_DEG
    dq func_rad,    3, FN_DEG_TO_RAD
    dq func_exp,    3, FN_EXPONENT_E
    dq func_csc,    3, FN_COSECANT
    dq func_sec,    3, FN_SECANT
    dq func_cot,    3, FN_COTANGENT
    dq 0, 0, 0              ; sentinel

; M_PI constant as double
align 8
dpi:    dq 0x400921FB54442D18    ; 3.14159265358979323846

; 180.0 as double
align 8
d180:   dq 0x4066800000000000

; 1.0 as double for reciprocal calculations
align 8
d1:     dq 0x3FF0000000000000

SECTION .data

; ---- Global mutable state ----
align 8
g_backup:       times 256 db 0     ; termios backup (oversized for safety)
g_retCode:      dd 0
g_globalFlags:  db 0
g_precision:    dq 6               ; default precision
g_prompt:       dq str_default_prompt
g_varCount:     dw 0
g_variables:    times (32 + 1) * 24 db 0  ; MAX_VARIABLES+1 slots, 24 bytes each

SECTION .bss

; ---- Constants section as equates (NASM allows in any section) ----
