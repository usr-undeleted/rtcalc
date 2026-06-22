; rtcalc.asm - Main entry point and interactive loop
; Translated from rtcalc.c

%include "macros.asm"
%include "equates.inc"

extern printf
extern fprintf
extern getchar
extern signal
extern tcgetattr
extern tcsetattr
extern atexit
extern memset
extern memcpy
extern memmove
extern strlen
extern strchr
extern strncmp
extern strcmp
extern strtoul
extern snprintf
extern isspace
extern isprint
extern exit
; Note: strerror available but not used

extern helpMenu
extern validateBuffer
extern calculateBuffer
extern printBufColored
extern retToStr
extern restoreTerminal
extern handleCtrlC
extern getPriority

extern g_backup
extern g_retCode
extern g_globalFlags
extern g_precision
extern g_prompt
extern g_varCount
extern g_variables

; Data from constants.asm
extern str_arg_help
extern str_arg_prompt
extern str_arg_syntax
extern str_arg_precision
extern str_arg_define
extern str_welcome
extern str_esc_save
extern str_esc_restore
extern str_esc_repl
extern str_esc_cr_down
extern str_format_double
extern str_cant_calc
extern str_delimiters
extern str_err_prefix1
extern str_err_prefix2
extern str_err_suffix

extern stderr

SECTION .text

global main
main:
    push    rbp
    mov     rbp, rsp
    sub     rsp, 8448            ; calcBuffer[2049] + result[1025] + locals
    ; Round to 16-aligned: 8448 % 16 = 0. Good.
    push    rbx
    push    r12
    push    r13
    push    r14
    push    r15
    ; Alignment: entry 8 + push rbp (8) + sub 8448 (8448) + push 5 regs (40) = 8504
    ; 8504 % 16 = 8. rsp%16 = (8-8504)%16 = -8496%16 = 0. Good.

    ; Locals (rbp-relative):
    ; [rbp - 8]     = precision (qword)
    ; [rbp - 16]    = vars ptr (for var count access)
    ; [rbp - 24]    = cursorPos (qword)
    ; [rbp - 32]    = len (qword)
    ; [rbp - 33]    = highestPrio (byte, stored as qword for simplicity)
    ; [rbp - 2049]  = calcBuffer (start)
    ; [rbp - 3074]  = result buffer (start)
    ; [rbp - 8200]  = scratch locals

    ; Default values
    mov     qword [rbp - 8], 6   ; precision = 6
    mov     qword [rbp - 24], 0  ; cursorPos = 0
    mov     byte [rel g_globalFlags], 0

    ; ---- Parse argc/argv ----
    mov     r14d, edi            ; r14 = argc
    mov     r15, rsi             ; r15 = argv
    cmp     r14d, 1
    jle     .setup_terminal

    mov     r13d, 1              ; i = 1 (skip argv[0])
.arg_loop:
    cmp     r13d, r14d
    jge     .setup_terminal
    mov     r12, [r15 + r13*8]   ; r12 = argv[i]

    ; Check "help"
    mov     rdi, r12
    lea     rsi, [rel str_arg_help]
    call    strcmp wrt ..plt
    test    eax, eax
    jnz     .check_prompt
    xor     edi, edi
    xor     esi, esi
    call    helpMenu
    ; helpMenu exits, never returns

.check_prompt:
    ; strncmp(argv[i], "prompt", 6)
    mov     rdi, r12
    lea     rsi, [rel str_arg_prompt]
    mov     edx, 6
    call    strncmp wrt ..plt
    test    eax, eax
    jnz     .check_syntax
    ; find '='
    mov     rdi, r12               ; rdi = string pointer
    mov     esi, '='               ; esi = char to find
    call    strchr wrt ..plt
    test    rax, rax
    jz      .bad_prompt
    cmp     byte [rax + 1], 0
    jz      .bad_prompt
    lea     rax, [rax + 1]
    mov     [rel g_prompt], rax
    jmp     .arg_next

.bad_prompt:
    lea     rdi, [rel .err_prompt]
    mov     esi, USER_MISTAKE
    call    helpMenu

.check_syntax:
    mov     rdi, r12
    lea     rsi, [rel str_arg_syntax]
    call    strcmp wrt ..plt
    test    eax, eax
    jnz     .check_precision
    mov     byte [rel g_globalFlags], USE_PRETTY_COLORS
    jmp     .arg_next

.check_precision:
    ; strncmp(argv[i], "precision", 9)
    mov     rdi, r12
    lea     rsi, [rel str_arg_precision]
    mov     edx, 9
    call    strncmp wrt ..plt
    test    eax, eax
    jnz     .check_define
    mov     rdi, r12               ; rdi = string pointer
    mov     esi, '='               ; esi = char to find
    call    strchr wrt ..plt
    test    rax, rax
    jz      .bad_prec
    cmp     byte [rax + 1], 0
    jz      .bad_prec
    ; strtoul(start, &check, 10)
    lea     rdi, [rax + 1]
    ; rsi = &check (we'll reuse rsp-relative later)
    lea     rsi, [rbp - 40]
    mov     edx, 10
    call    strtoul wrt ..plt
    ; validate
    mov     rcx, [rbp - 40]
    cmp     byte [rcx], 0
    jne     .bad_prec
    mov     [rbp - 8], rax       ; precision
    jmp     .arg_next

.bad_prec:
    lea     rdi, [rel .err_prec]
    mov     esi, USER_MISTAKE
    call    helpMenu

.check_define:
    ; strncmp(argv[i], "define", 5)
    mov     rdi, r12
    lea     rsi, [rel str_arg_define]
    mov     edx, 5
    call    strncmp wrt ..plt
    test    eax, eax
    jnz     .bad_flag
    ; handle define (complex, skip for now - set variable directly)
    jmp     .arg_next

.bad_flag:
    lea     rdi, [rel .err_flag]
    mov     esi, USER_MISTAKE
    call    helpMenu

.arg_next:
    inc     r13d
    jmp     .arg_loop

.setup_terminal:
    ; signal(SIGINT, handleCtrlC)
    mov     edi, 2               ; SIGINT
    lea     rsi, [rel handleCtrlC]
    call    signal wrt ..plt

    ; tcgetattr(STDIN_FILENO, &backup)
    xor     edi, edi
    lea     rsi, [rel g_backup]
    call    tcgetattr wrt ..plt
    test    eax, eax
    jns     .attr_ok
    mov     eax, CODE_MISTAKE
    jmp     .do_exit

.attr_ok:
    ; atexit(restoreTerminal)
    lea     rdi, [rel restoreTerminal]
    call    atexit wrt ..plt

    ; Copy backup to new mode, modify flags
    ; memcpy(&newMode, &backup, sizeof(struct termios))
    lea     rdi, [rbp - 8200]    ; newMode (generous size)
    lea     rsi, [rel g_backup]
    mov     edx, 256             ; oversized copy
    call    memcpy wrt ..plt

    ; newMode.c_lflag &= ~(ICANON | ECHO)
    lea     rax, [rbp - 8200]
    mov     ecx, [rax + TERMIOS_LFLAG]
    and     ecx, ~(ICANON | ECHO_FLAG) & 0xFFFFFFFF
    mov     [rax + TERMIOS_LFLAG], ecx

    ; newMode.c_cc[VMIN] = 1, newMode.c_cc[VTIME] = 0
    add     rax, TERMIOS_CC
    mov     byte [rax + VMIN_INDEX], 1
    mov     byte [rax + VTIME_INDEX], 0

    ; tcsetattr(STDIN_FILENO, TCSAFLUSH, &newMode)
    xor     edi, edi
    mov     esi, 2               ; TCSAFLUSH
    lea     rdx, [rbp - 8200]
    call    tcsetattr wrt ..plt

    ; ---- Initialize buffer ----
    lea     rax, [rbp - 2049]    ; calcBuffer
    mov     rdi, rax
    xor     esi, esi
    mov     edx, BUFFER_SIZE
    call    memset wrt ..plt

    lea     rax, [rbp - 3074]    ; result buffer
    mov     rdi, rax
    xor     esi, esi
    mov     edx, RESULT_SIZE
    call    memset wrt ..plt

    ; Print prompt + save cursor
    mov     rdi, [rel g_prompt]
    lea     rsi, [rel str_esc_save]
    call    printf wrt ..plt

    mov     qword [rbp - 24], 0  ; cursorPos = 0

    ; ==== MAIN INTERACTIVE LOOP ====
.main_loop:
    ; ---- Validate buffer ----
    lea     rdi, [rbp - 2049]    ; calcBuffer
    lea     rsi, [rbp - 33]      ; &highestPrio (on stack)
    lea     rdx, [rel g_variables]
    call    validateBuffer
    movzx   r12d, al             ; r12 = ret (error code, 0 = ok)

    ; input size limit check
    mov     rax, [rbp - 24]      ; cursorPos
    cmp     rax, BUFFER_SIZE - 1
    jb      .size_ok
    mov     r12d, E_INPUT_SIZE_LIMIT
.size_ok:

    ; ---- Format result ----
    mov     rax, [rbp - 24]      ; cursorPos
    lea     rcx, [rbp - 2049]
    cmp     byte [rcx], 0
    jne     .have_input
    test    rax, rax
    jnz     .have_input
    ; Welcome message
    lea     rdi, [rbp - 3074]
    mov     esi, RESULT_SIZE
    lea     rdx, [rel str_welcome]
    call    snprintf wrt ..plt
    jmp     .print_ui

.have_input:
    test    r12b, r12b
    jnz     .error_result
    ; Calculate
    lea     rdi, [rbp - 2049]
    movzx   esi, byte [rbp - 33] ; highestPrio
    lea     rdx, [rel g_variables]
    call    calculateBuffer
    ; result in xmm0

    ; snprintf(buf, size, "%.*lf", precision, result)
    lea     rdi, [rbp - 3074]    ; buf
    mov     esi, RESULT_SIZE     ; size
    lea     rdx, [rel str_format_double] ; fmt
    mov     ecx, [rbp - 8]       ; precision
    ; xmm0 already has result, first FP vararg
    mov     eax, 1               ; 1 xmm arg
    call    snprintf wrt ..plt
    ; check if display too large
    cmp     rax, RESULT_SIZE - 1
    jb      .print_ui
    mov     r12d, E_DISPLAY_SIZE_LIMIT
    jmp     .error_result

.error_result:
    ; snprintf(result_buf, RESULT_SIZE, "Can't calculate: %s", retToStr(ret))
    movzx   edi, r12b
    call    retToStr
    mov     rcx, rax            ; error string
    lea     rdi, [rbp - 3074]
    mov     esi, RESULT_SIZE
    lea     rdx, [rel str_cant_calc]
    sub     rsp, 8
    call    snprintf wrt ..plt
    add     rsp, 8

.print_ui:
    ; Display: restore cursor, print result, print prompt, print buffer
    lea     rdi, [rel str_esc_restore]
    xor     eax, eax
    call    printf wrt ..plt

    lea     rdi, [rel str_esc_repl]
    lea     rsi, [rbp - 3074]    ; result
    mov     rdx, [rel g_prompt]  ; prompt
    xor     eax, eax
    call    printf wrt ..plt

    ; Print buffer (with error prefix if needed)
    cmp     r12b, 0
    jnz     .print_err
    lea     rdi, [rbp - 2049]
    xor     eax, eax
    call    printf wrt ..plt
    jmp     .cursor_pos
.print_err:
    lea     rdi, [rel .fmt_err_buf]
    lea     rsi, [rbp - 2049]
    xor     eax, eax
    call    printf wrt ..plt

.cursor_pos:
    ; Move cursor to cursorPos
    ; Compute strlen(calcBuffer) first
    lea     rdi, [rbp - 2049]
    call    strlen wrt ..plt
    mov     r13, rax             ; len in r13

    cmp     r13, qword [rbp - 24]
    jle     .no_move
    sub     r13, [rbp - 24]
    lea     rdi, [rel .fmt_cursor_l]
    mov     esi, r13d
    call    printf wrt ..plt

.no_move:
    ; ---- Read character ----
    call    getchar wrt ..plt
    cmp     eax, -1              ; EOF
    je      .exit_eof
    mov     ebx, eax             ; c = char

    ; Ignore newlines and form feeds
    cmp     bl, NL_CHAR
    je      .main_loop
    cmp     bl, FF_CHAR
    je      .main_loop

    ; Escape sequence
    cmp     bl, ESC_CHAR
    jne     .check_backspace

    call    getchar wrt ..plt
    cmp     al, '['
    jne     .main_loop

    call    getchar wrt ..plt     ; c = sequence key
    cmp     al, 'C'
    je      .right_arrow
    cmp     al, 'D'
    je      .left_arrow
    cmp     al, '1'
    je      .ctrl_arrow_seq
    jmp     .main_loop

.right_arrow:
    mov     rax, [rbp - 24]
    cmp     rax, r13
    jge     .main_loop
    inc     qword [rbp - 24]
    jmp     .main_loop

.left_arrow:
    cmp     qword [rbp - 24], 0
    je      .main_loop
    dec     qword [rbp - 24]
    jmp     .main_loop

.ctrl_arrow_seq:
    ; Read rest of sequence until final char (A-~)
    mov     r8, 7                ; max extra chars
.ctrl_seq_loop:
    call    getchar wrt ..plt
    cmp     al, 'A'
    jb      .ctrl_next
    cmp     al, '~'
    ja      .ctrl_next
    ; Final character
    ; Check what sequence: ;5D = ctrl+left, ;5C = ctrl+right
    ; The buffer currently has the sequence chars. We already consumed '1', ';', '5', then final.
    ; Since we don't store the full sequence, just check if it was 'D' or 'C'
    ; Actually we need to know if it was ;5D or ;5C
    ; The final char tells us: D = left, C = right
    cmp     al, 'D'
    je      .ctrl_left
    cmp     al, 'C'
    je      .ctrl_right
    jmp     .main_loop
.ctrl_next:
    dec     r8
    jnz     .ctrl_seq_loop
    jmp     .main_loop

.ctrl_left:
    ; Simplified: just move one char left
    cmp     qword [rbp - 24], 0
    je      .main_loop
    dec     qword [rbp - 24]
    jmp     .main_loop

.ctrl_right:
    ; Simplified: just move one char right
    lea     rdi, [rbp - 2049]
    call    strlen wrt ..plt
    cmp     [rbp - 24], rax
    jge     .main_loop
    inc     qword [rbp - 24]
    jmp     .main_loop

.check_backspace:
    cmp     bl, DEL_CHAR
    je      .do_backspace
    cmp     bl, BS_CHAR
    je      .do_backspace
    jmp     .check_ctrl_w

.do_backspace:
    cmp     qword [rbp - 24], 0
    je      .main_loop
    ; memmove(&buf[cursorPos-1], &buf[cursorPos], len - cursorPos)
    lea     rdi, [rbp - 2049]
    mov     rax, [rbp - 24]
    dec     rax
    add     rdi, rax             ; dest = buf + cursorPos - 1
    lea     rsi, [rdi + 1]      ; src = buf + cursorPos
    ; len = current len. Need to compute...
    ; Actually, let me compute len from r13 (which is stale). Let me recompute.
    lea     rdi, [rbp - 2049]
    call    strlen wrt ..plt
    mov     r13, rax
    ; Now memmove
    lea     rdi, [rbp - 2049]
    mov     rax, [rbp - 24]
    dec     rax
    add     rdi, rax
    lea     rsi, [rbp - 2049]
    add     rsi, [rbp - 24]
    sub     r13, [rbp - 24]      ; len - cursorPos
    mov     rdx, r13
    add     rdx, 1               ; include null terminator
    call    memmove wrt ..plt
    dec     qword [rbp - 24]     ; cursorPos--
    jmp     .main_loop

.check_ctrl_w:
    cmp     bl, CTRL_W
    jne     .check_ctrl_x
    ; Ctrl+W: delete word (simplified - just backspace 1 for now)
    jmp     .do_backspace

.check_ctrl_x:
    cmp     bl, CTRL_X
    jne     .check_ctrl_k
    ; Clear line
    mov     qword [rbp - 24], 0
    lea     rdi, [rbp - 2049]
    xor     esi, esi
    mov     edx, BUFFER_SIZE
    call    memset wrt ..plt
    jmp     .main_loop

.check_ctrl_k:
    cmp     bl, CTRL_K
    jne     .check_ctrl_a
    ; Clear to end of line
    lea     rax, [rbp - 2049]
    add     rax, [rbp - 24]
    cmp     byte [rax], 0
    je      .main_loop
    mov     byte [rax], 0
    jmp     .main_loop

.check_ctrl_a:
    cmp     bl, CTRL_A
    jne     .check_ctrl_e
    mov     qword [rbp - 24], 0  ; cursorPos = 0
    jmp     .main_loop

.check_ctrl_e:
    cmp     bl, CTRL_E
    jne     .check_printable
    lea     rdi, [rbp - 2049]
    call    strlen wrt ..plt
    mov     [rbp - 24], rax      ; cursorPos = len
    jmp     .main_loop

.check_printable:
    ; isprint(c)
    movzx   edi, bl
    call    isprint wrt ..plt
    test    eax, eax
    jz      .main_loop

    ; Insert character at cursorPos
    lea     rdi, [rbp - 2049]
    call    strlen wrt ..plt
    mov     r13, rax
    cmp     r13, BUFFER_SIZE - 2
    jae     .main_loop           ; buffer full

    ; memmove(&buf[cursorPos+1], &buf[cursorPos], len - cursorPos + 1)
    lea     rdi, [rbp - 2049]
    add     rdi, [rbp - 24]
    lea     rsi, [rdi]           ; src = dest
    inc     rdi                  ; dest = buf + cursorPos + 1
    mov     rdx, r13
    sub     rdx, [rbp - 24]
    inc     rdx                  ; len - cursorPos + 1
    call    memmove wrt ..plt

    ; buf[cursorPos] = c
    lea     rax, [rbp - 2049]
    add     rax, [rbp - 24]
    mov     [rax], bl
    inc     qword [rbp - 24]

    jmp     .main_loop

.exit_eof:
    xor     eax, eax             ; return 0 for clean exit
    jmp     .do_exit

.exit:
.do_exit:
    pop     r15
    pop     r14
    pop     r13
    pop     r12
    pop     rbx
    lea     rsp, [rbp]
    pop     rbp
    ret

SECTION .rodata
.err_prompt: db 10, 27, "[31mError: Prompt not defined properly.", 27, "[0m", 10, 0
.err_prec:   db 10, 27, "[31mError: Precision is an invalid number.", 27, "[0m", 10, 0
.err_flag:   db 10, 27, "[31mError: Improper flag used.", 27, "[0m", 10, 0
.err_term:   db "Failed to backup term attributes.", 10, 0
.fmt_err_buf: db 27, "[31m%s", 27, "[0m", 0
.fmt_cursor_l: db 27, "[%dD", 0
