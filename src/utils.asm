; utils.asm - Utility functions for rtcalc
; Translated from utilities.h

%include "macros.asm"
%include "equates.inc"

extern strchr
extern strncmp
extern printf
extern exit
extern fprintf
extern tcsetattr
extern pow
extern fmod

; Data symbols from constants.asm
extern str_help_header, str_help_legal, str_help_usage
extern str_help_details, str_help_args
extern str_version
extern func_table
extern err_unknown, err_invalid_char, err_insuf_close_paren, err_insuf_open_paren
extern err_invalid_operand, err_invalid_operator, err_insuf_nums, err_insuf_ops
extern err_empty_paren, err_func_invalid_brackets, err_empty_function
extern err_var_open_brackets, err_var_unknown
extern err_multi_arg_insuf, err_multi_arg_excess
extern err_multi_arg_invalid_first, err_multi_arg_invalid_second
extern err_display_size_limit, err_input_size_limit
extern g_backup, g_retCode
extern getPriority

extern stderr

SECTION .text

; ---- skipWhitespace(const char **pp) ----
; rdi = pp (char**)
; Advances *pp past leading whitespace
skipWhitespace:
    mov     rax, [rdi]          ; rax = *pp
.loop:
    movzx   ecx, byte [rax]
    test    cl, cl
    jz      .done
    cmp     cl, ' '
    je      .skip
    cmp     cl, 9               ; tab
    jb      .done
    cmp     cl, 13              ; CR
    ja      .done
.skip:
    inc     rax
    jmp     .loop
.done:
    mov     [rdi], rax
    ret

; ---- uint8_t getPriority(char op) ----
; dil = op
; Returns priority in al
getPriority:
    movzx   eax, dil
    cmp     al, '+'
    je      .p1
    cmp     al, '-'
    je      .p1
    cmp     al, '*'
    je      .p2
    cmp     al, '/'
    je      .p2
    cmp     al, '%'
    je      .p2
    cmp     al, '^'
    je      .p3
    xor     eax, eax
    ret
.p1:
    mov     eax, 1
    ret
.p2:
    mov     eax, 2
    ret
.p3:
    mov     eax, 3
    ret

; ---- int getFuncIndex(const char *ptr) ----
; rdi = ptr
; Returns enum value in eax, or -1 if not found
global getFuncIndex
getFuncIndex:
    push    rbx
    push    r12
    push    r13
    push    r14
    sub     rsp, 8              ; align stack for calls
    mov     rbx, rdi            ; rbx = ptr
    lea     r12, [rel func_table]
.loop:
    mov     r13, [r12]          ; r13 = name_ptr
    test    r13, r13
    jz      .not_found
    mov     r14, [r12 + 8]      ; r14 = name_len
    ; strncmp(ptr, name, len)
    mov     rdi, rbx
    mov     rsi, r13
    mov     rdx, r14
    call    strncmp wrt ..plt
    test    eax, eax
    jz      .found
    add     r12, 24             ; next entry (3 * 8 bytes)
    jmp     .loop
.found:
    mov     rax, [r12 + 16]     ; return enum value
    add     rsp, 8
    pop     r14
    pop     r13
    pop     r12
    pop     rbx
    ret
.not_found:
    mov     eax, -1
    add     rsp, 8
    pop     r14
    pop     r13
    pop     r12
    pop     rbx
    ret

; ---- double calculateTrio(double left, char op, double right) ----
; xmm0 = left, edi = op, xmm1 = right
; Returns result in xmm0
global calculateTrio
calculateTrio:
    movzx   eax, dil
    cmp     al, '+'
    je      .add
    cmp     al, '-'
    je      .sub
    cmp     al, '*'
    je      .mul
    cmp     al, '/'
    je      .div
    cmp     al, '^'
    je      .pow_fn
    cmp     al, '%'
    je      .fmod_fn
    xorps   xmm0, xmm0          ; default: return 0
    ret
.add:
    addsd   xmm0, xmm1
    ret
.sub:
    subsd   xmm0, xmm1
    ret
.mul:
    mulsd   xmm0, xmm1
    ret
.div:
    divsd   xmm0, xmm1
    ret
.pow_fn:
    sub     rsp, 8              ; align stack
    call    pow wrt ..plt       ; pow(xmm0, xmm1)
    add     rsp, 8
    ret
.fmod_fn:
    sub     rsp, 8
    call    fmod wrt ..plt      ; fmod(xmm0, xmm1)
    add     rsp, 8
    ret

; ---- char *findFuncClose(const char *ptr, char **openOut, int *errCode) ----
; rdi = ptr, rsi = openOut (may be NULL), rdx = errCode (may be NULL)
; Returns close ptr in rax, or NULL on error
global findFuncClose
findFuncClose:
    push    rbx
    push    r12
    push    r13
    push    r14
    push    r15
    mov     r12, rsi            ; openOut
    mov     r13, rdx            ; errCode
    ; strchr(ptr, '[')
    mov     rsi, '['
    call    strchr wrt ..plt
    test    rax, rax
    jz      .err
    mov     rbx, rax            ; rbx = open
    mov     r14, rax            ; r14 = close (init to open)
    mov     r15, rax            ; r15 = validateBracket
    xor     r8, r8              ; r8 = depth
.loop:
    movzx   r9d, byte [r15]
    test    r9, r9
    jz      .check
    cmp     r9, '['
    jne     .try_close
    inc     r8
    jmp     .advance
.try_close:
    cmp     r9, ']'
    jne     .advance
    mov     r14, r15            ; close = validateBracket
    test    r8, r8
    jz      .advance
    dec     r8
.advance:
    inc     r15
    test    r8, r8
    jnz     .loop
    ; depth == 0, fall through
.check:
    test    r8, r8
    jnz     .err
    cmp     r14, rbx            ; close == open?
    je      .err
    test    r12, r12
    jz      .ret
    mov     [r12], rbx          ; *openOut = open
.ret:
    mov     rax, r14
    pop     r15
    pop     r14
    pop     r13
    pop     r12
    pop     rbx
    ret
.err:
    test    r13, r13
    jz      .err_ret
    mov     dword [r13], E_FUNC_INVALID_BRACKETS
.err_ret:
    xor     rax, rax
    pop     r15
    pop     r14
    pop     r13
    pop     r12
    pop     rbx
    ret

; ---- char *findVarClose(const char *ptr) ----
; rdi = ptr
; Returns close ptr in rax, or NULL on error
global findVarClose
findVarClose:
    push    rbx
    push    r12
    push    r13
    push    r14
    sub     rsp, 8              ; align stack
    ; strchr(ptr, '{')
    mov     rsi, '{'
    call    strchr wrt ..plt
    test    rax, rax
    jz      .err
    mov     rbx, rax            ; open
    mov     r12, rax            ; close
    mov     r13, rax            ; validateBracket
    xor     r8, r8              ; depth
.loop:
    movzx   r9d, byte [r13]
    test    r9, r9
    jz      .check
    cmp     r9, '{'
    jne     .try_close
    inc     r8
    jmp     .advance
.try_close:
    cmp     r9, '}'
    jne     .advance
    mov     r12, r13            ; close = validateBracket
    test    r8, r8
    jz      .advance
    dec     r8
.advance:
    inc     r13
    test    r8, r8
    jnz     .loop
.check:
    test    r8, r8
    jnz     .err
    cmp     r12, rbx
    je      .err
    mov     rax, r12
    add     rsp, 8
    pop     r14
    pop     r13
    pop     r12
    pop     rbx
    ret
.err:
    xor     rax, rax
    add     rsp, 8
    pop     r14
    pop     r13
    pop     r12
    pop     rbx
    ret

; ---- char *findFuncComma(const char *buf, const char *end) ----
; rdi = buf, rsi = end
; Returns pointer to comma in rax, or NULL
global findFuncComma
findFuncComma:
    mov     rax, rdi            ; ptr = buf
    xor     r8, r8              ; depth = 0
.loop:
    cmp     rax, rsi            ; ptr == end?
    je      .not_found
    movzx   r9d, byte [rax]
    cmp     r9, '('
    je      .inc_depth
    cmp     r9, '['
    je      .inc_depth
    cmp     r9, '{'
    je      .inc_depth
    cmp     r9, ')'
    je      .dec_depth
    cmp     r9, ']'
    je      .dec_depth
    cmp     r9, '}'
    je      .dec_depth
    cmp     r9, ','
    jne     .next
    test    r8, r8
    jnz     .next              ; depth != 0, skip
    ret                       ; return ptr (comma at depth 0)
.inc_depth:
    inc     r8
    jmp     .next
.dec_depth:
    dec     r8
.next:
    inc     rax
    jmp     .loop
.not_found:
    xor     rax, rax
    ret

; ---- char *retToStr(char err) ----
; dil = err code
; Returns string pointer in rax
global retToStr
retToStr:
    movzx   eax, dil
    lea     rcx, [rel .err_table]
    ; bounds check
    cmp     eax, E_INPUT_SIZE_LIMIT
    ja      .unknown
    mov     rax, [rcx + rax * 8]
    ret
.unknown:
    lea     rax, [rel err_unknown]
    ret

SECTION .rodata
.err_table:
    dq err_unknown                          ; E_SEEMINGLY_OKAY = 0 (unused, default to unknown)
    dq err_invalid_char                     ; 1
    dq err_insuf_close_paren                ; 2
    dq err_insuf_open_paren                 ; 3
    dq err_empty_paren                      ; 4
    dq err_invalid_operand                  ; 5
    dq err_invalid_operator                 ; 6
    dq err_insuf_nums                       ; 7
    dq err_insuf_ops                        ; 8
    dq err_func_invalid_brackets            ; 9
    dq err_empty_function                   ; 10
    dq err_var_open_brackets                ; 11
    dq err_var_unknown                      ; 12
    dq err_multi_arg_insuf                  ; 13
    dq err_multi_arg_excess                 ; 14
    dq err_multi_arg_invalid_first          ; 15
    dq err_multi_arg_invalid_second         ; 16
    dq err_display_size_limit               ; 17
    dq err_input_size_limit                 ; 18

SECTION .text

; ---- void helpMenu(char *error, int ret) ----
; rdi = error string (may be NULL), rsi = ret code
global helpMenu
helpMenu:
    push    rbx
    push    r12
    mov     rbx, rdi            ; error
    mov     r12, rsi            ; ret code
    sub     rsp, 8              ; align for calls

    ; Print header
    lea     rdi, [rel str_help_header]
    xor     eax, eax
    call    printf wrt ..plt

    ; Print legal info (has %s for version)
    lea     rdi, [rel str_help_legal]
    lea     rsi, [rel str_version]
    xor     eax, eax
    call    printf wrt ..plt

    ; Print usage
    lea     rdi, [rel str_help_usage]
    xor     eax, eax
    call    printf wrt ..plt

    ; Print details (has %zu for BUFFER_SIZE-1 and RESULT_SIZE-1)
    lea     rdi, [rel str_help_details]
    mov     rsi, BUFFER_SIZE - 1
    mov     rdx, RESULT_SIZE - 1
    xor     eax, eax
    call    printf wrt ..plt

    ; Print args (has %s for error or empty)
    lea     rdi, [rel str_help_args]
    test    rbx, rbx
    jnz     .have_error
    xor     esi, esi
    jmp     .print_args
.have_error:
    mov     rsi, rbx
.print_args:
    xor     eax, eax
    call    printf wrt ..plt

    ; exit(ret)
    mov     edi, r12d
    call    exit wrt ..plt
    ; exit never returns
    add     rsp, 8
    pop     r12
    pop     rbx
    ret

; ---- void handleCtrlC(int sig) ----
; edi = signal number
global handleCtrlC
handleCtrlC:
    sub     rsp, 8
    ; fprintf(stderr, "\nInterrupted, exiting...\n")
    mov     rdi, [rel stderr]
    lea     rsi, [rel .msg]
    xor     eax, eax
    call    fprintf wrt ..plt
    ; exit(retCode)
    mov     edi, [rel g_retCode]
    call    exit wrt ..plt
    add     rsp, 8
    ret

SECTION .rodata
.msg: db 10, "Interrupted, exiting...", 10, 0

SECTION .text

; ---- void restoreTerminal(void) ----
global restoreTerminal
restoreTerminal:
    push    rbx                 ; rsp now 16-aligned
    ; tcsetattr(STDIN_FILENO, TCSAFLUSH, &backup)
    mov     edi, 0              ; STDIN_FILENO
    mov     esi, 2              ; TCSAFLUSH
    lea     rdx, [rel g_backup]
    call    tcsetattr wrt ..plt
    pop     rbx
    ret
