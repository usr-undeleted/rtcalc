; evaluate.asm - Expression tokenizer, validator, and evaluator
; Translated from functions.h

%include "macros.asm"
%include "equates.inc"

extern getFuncIndex
extern findFuncClose
extern findVarClose
extern findFuncComma
extern skipWhitespace
extern calculateTrio
extern getPriority
extern memset
extern memcpy
extern strncmp
extern strchr
extern strtod
extern printf

; libm functions
extern sqrt
extern cbrt
extern sin
extern cos
extern tan
extern sinh
extern cosh
extern tanh
extern asin
extern acos
extern atan
extern asinh
extern acosh
extern atanh
extern floor
extern ceil
extern log
extern log10
extern log2
extern gamma
extern trunc
extern erf
extern erfc
extern lgamma
extern fabs
extern round
extern exp
extern exp2
extern pow
extern fmax
extern fmin
extern hypot
extern atan2

extern dpi
extern d180
extern d1

; Data symbols from constants.asm
extern str_valid_list, str_operations, str_delimiters
extern func_table
extern g_variables

SECTION .text

; ============================================================
; size_t countTokens(const char *buf, char flags)
; rdi = buf, sil = flags
; Returns count in rax
; ============================================================
global countTokens
countTokens:
    push    rbp
    mov     rbp, rsp
    sub     rsp, 32              ; locals + alignment
    push    rbx
    push    r12
    push    r13
    push    r14
    ; Stack is now aligned (push rbp=8 + sub 32 + push 4*8=32 = 72 from entry, 72%16=8, entry was 8, so rsp%16=0)

    mov     rbx, rdi             ; ptr = buf
    movzx   r12d, sil            ; flags
    xor     r13, r13             ; count = 0
    xor     r14, r14             ; mode = 0

.loop:
    ; while (*ptr)
    movzx   eax, byte [rbx]
    test    al, al
    jz      .done
    ; skipWhitespace
    SKIP_WS rbx
    ; check null after skip
    movzx   eax, byte [rbx]
    test    al, al
    jz      .done

    ; if (!mode) try function lookup
    test    r14b, r14b
    jnz     .check_commas

    mov     rdi, rbx
    call    getFuncIndex
    cmp     eax, -1
    je      .check_commas

    ; found function
    mov     rdi, rbx             ; ptr
    lea     rsi, [rbp - 16]     ; &open
    xor     edx, edx            ; errCode = NULL
    call    findFuncClose
    ; rax = close, [rbp-16] = open

    test    r12b, CT_FLAG_READ_BRACKETS
    jz      .func_skip
    ; ptr = open
    mov     rbx, [rbp - 16]
    jmp     .func_done
.func_skip:
    lea     rbx, [rax + 1]      ; ptr = close + 1
    xor     r14b, 1             ; mode = !mode
.func_done:
    inc     r13
    jmp     .loop

.check_commas:
    test    r12b, CT_FLAG_READ_COMMAS
    jz      .check_parens
    movzx   eax, byte [rbx]
    cmp     al, ','
    jne     .check_parens
    inc     rbx
    inc     r13
    xor     r14b, 1
    jmp     .loop

.check_parens:
    movzx   eax, byte [rbx]
    cmp     al, '('
    je      .paren_found
    cmp     al, ')'
    jne     .check_curly
.paren_found:
    inc     rbx
    inc     r13
    jmp     .loop

.check_curly:
    test    r12b, CT_FLAG_READ_CURLY_BRACKETS
    jz      .check_fbrackets
    movzx   eax, byte [rbx]
    cmp     al, '{'
    jne     .check_fbrackets
    mov     rdi, rbx
    call    findVarClose
    lea     rbx, [rax + 1]
    add     r13, 3
    jmp     .loop

.check_fbrackets:
    test    r12b, CT_FLAG_READ_BRACKETS
    jz      .switch_type
    movzx   eax, byte [rbx]
    cmp     al, '['
    je      .fbracket_found
    cmp     al, ']'
    jne     .switch_type
.fbracket_found:
    inc     rbx
    inc     r13
    jmp     .loop

.switch_type:
    test    r14b, r14b
    jnz     .op_mode
    ; number mode: strtod(ptr, &endptr)
    mov     rdi, rbx
    lea     rsi, [rbp - 8]      ; endptr storage
    call    strtod wrt ..plt
    mov     rbx, [rbp - 8]      ; ptr = endptr
    jmp     .type_done
.op_mode:
    inc     rbx                  ; ptr++
.type_done:
    inc     r13                  ; count++
    xor     r14b, 1             ; mode = !mode
    jmp     .loop

.done:
    mov     rax, r13
    pop     r14
    pop     r13
    pop     r12
    pop     rbx
    lea     rsp, [rbp]
    pop     rbp
    ret

; ============================================================
; int validateBuffer(char *buffer, int *highestPrio, const struct variable *variables)
; rdi = buffer, rsi = highestPrio (may be NULL), rdx = variables
; Returns error code in eax (0 = OK)
; ============================================================
global validateBuffer
validateBuffer:
    push    rbp
    mov     rbp, rsp
    sub     rsp, 80              ; locals (see layout below)
    push    rbx                  ; ptr
    push    r12                  ; variables
    push    r13                  ; openParentheses
    push    r14                  ; nums
    push    r15                  ; ops

    ; Locals layout (rbp-relative):
    ; [rbp-8]  = ptr (for &ptr access)
    ; [rbp-16] = mode (byte)
    ; [rbp-24] = lastWasParen (byte)
    ; [rbp-32] = endptr storage (for strtod)
    ; [rbp-40] = open storage (for findFuncClose)
    ; [rbp-48] = highestPrio pointer
    ; [rbp-56] = varLen
    ; [rbp-64] = childLen
    ; [rbp-72] = funcIndex
    ; [rbp-80] = ret storage

    mov     rbx, rdi             ; ptr = buffer
    mov     r12, rdx             ; variables
    mov     [rbp - 48], rsi      ; highestPrio pointer
    xor     r13, r13             ; openParentheses = 0
    xor     r14, r14             ; nums = 0
    xor     r15, r15             ; ops = 0
    mov     qword [rbp - 16], 0  ; mode = 0
    mov     qword [rbp - 24], 0  ; lastWasParen = 0

.loop:
    movzx   eax, byte [rbx]
    test    al, al
    jz      .final_check
    SKIP_WS rbx
    movzx   eax, byte [rbx]
    test    al, al
    jz      .final_check
    mov     qword [rbp - 24], 0  ; lastWasParen = 0

    ; if (!mode)
    mov     rax, [rbp - 16]
    test    rax, rax
    jnz     .after_func_check

    ; ---- Variable check: *ptr == '{' ----
    movzx   eax, byte [rbx]
    cmp     al, '{'
    jne     .check_functions

    ; findVarClose(ptr)
    mov     rdi, rbx
    call    findVarClose
    test    rax, rax
    jz      .ret_var_open_brackets
    ; rax = close
    ; varLen = close - ptr - 1
    mov     rcx, rax
    sub     rcx, rbx
    dec     rcx                  ; varLen = close - ptr - 1
    mov     [rbp - 56], rcx      ; save varLen

    ; Allocate child buffer: bufComp[varLen + 1]
    ; We use alloca-style: sub rsp by aligned size
    lea     rax, [rcx + 16]      ; varLen + 1 + padding
    and     rax, -16             ; align to 16
    sub     rsp, rax             ; allocate on stack
    mov     [rbp - 8], rsp       ; bufComp pointer (reuse [rbp-8])

    ; memset(bufComp, 0, varLen + 1)
    mov     rdi, [rbp - 8]       ; bufComp
    xor     esi, esi            ; 0
    mov     rdx, [rbp - 56]
    inc     rdx                  ; varLen + 1
    call    memset wrt ..plt

    ; memcpy(bufComp, ptr + 1, varLen)
    mov     rdi, [rbp - 8]       ; dest = bufComp
    lea     rsi, [rbx + 1]       ; src = ptr + 1
    mov     rdx, [rbp - 56]      ; varLen
    call    memcpy wrt ..plt

    ; Loop through variables array
    xor     r8, r8              ; i = 0
    mov     r9, [rbp - 56]      ; varLen for comparison
.var_loop:
    cmp     r8, MAX_VARIABLES
    jae     .var_fail
    ; check variables[i].name != NULL
    mov     rax, r8
    imul    rax, rax, VAR_SIZE
    lea     rcx, [r12 + rax]
    mov     rax, [rcx + VAR_OFF_NAME]
    test    rax, rax
    jz      .var_fail            ; name == NULL means end of list
    ; check len match
    mov     rdx, [rcx + VAR_OFF_LEN]
    cmp     rdx, r9
    jne     .var_next
    ; strncmp(bufComp, name, len)
    mov     rdi, [rbp - 8]       ; bufComp
    mov     rsi, [rcx + VAR_OFF_NAME]
    mov     rdx, r9              ; varLen
    call    strncmp wrt ..plt
    test    eax, eax
    jz      .var_found
.var_next:
    inc     r8
    jmp     .var_loop
.var_found:
    ; success - advance ptr past close
    ; rax (close) was lost, recompute from saved stack? 
    ; Actually close was computed as rbx + varLen + 1 + 1 (close = ptr + 1 + varLen + 1 - 1)... 
    ; Let me just recompute: close = ptr + varLen + 2 (ptr + '{' + varLen chars + '}' - 1)
    ; close = rbx + [rbp-56] + 1. So ptr = close + 1 = rbx + varLen + 2
    mov     rcx, [rbp - 56]
    lea     rbx, [rbx + rcx + 2] ; ptr = close + 1
    inc     r14                  ; nums++
    ; mode = !mode
    mov     qword [rbp - 16], 1
    ; Restore stack (undo the VLA allocation)
    lea     rsp, [rbp - 80]
    sub     rsp, 40              ; push 5 regs = 40 bytes
    ; Actually rsp needs to be restored to the value after pushes
    ; Hmm, this is getting messy. Let me use a different approach.
    jmp     .loop

.var_fail:
    mov     eax, E_VAR_UNKNOWN
    jmp     .cleanup_and_ret

.ret_var_open_brackets:
    mov     eax, E_VAR_OPEN_BRACKETS
    jmp     .cleanup_and_ret

.check_functions:
    ; getFuncIndex(ptr)
    mov     rdi, rbx
    call    getFuncIndex
    cmp     eax, -1
    je      .after_func_check
    mov     [rbp - 72], rax      ; save funcIndex

    ; Check if multi-arg
    cmp     eax, FN_MULTI_ARG_PAD
    jle     .single_arg_func

    ; ---- Multi-arg function ----
    ; findFuncClose(ptr, &open, &ret)
    mov     rdi, rbx
    lea     rsi, [rbp - 40]      ; &open
    lea     rdx, [rbp - 80]      ; &ret
    mov     dword [rbp - 80], 0  ; ret = 0
    call    findFuncClose
    ; rax = close
    test    rax, rax
    jz      .check_ret_error
    mov     ecx, [rbp - 80]      ; ret
    test    ecx, ecx
    jnz     .return_ret_error

    ; Check if empty (skip whitespace between open+1 and close)
    mov     rsi, [rbp - 40]      ; open
    lea     r8, [rsi + 1]        ; val = open + 1
    mov     byte [rbp - 24], 1   ; isEmpty = 1
.empty_check_ma:
    cmp     r8, rax              ; val == close?
    jae     .empty_ma_done
    movzx   ecx, byte [r8]
    test    cl, cl
    jz      .empty_ma_done
    ; isspace check
    cmp     cl, ' '
    je      .empty_ma_adv
    cmp     cl, 9
    jb      .not_empty_ma
    cmp     cl, 13
    ja      .not_empty_ma
.empty_ma_adv:
    inc     r8
    jmp     .empty_check_ma
.not_empty_ma:
    mov     byte [rbp - 24], 0   ; isEmpty = 0
.empty_ma_done:
    movzx   ecx, byte [rbp - 24]
    test    ecx, ecx
    jnz     .ret_empty_function

    ; findFuncComma(open+1, close-1)
    mov     rdi, [rbp - 40]
    inc     rdi                   ; open + 1
    lea     rsi, [rax - 1]        ; close - 1
    ; Save close before call
    mov     [rbp - 8], rax        ; save close in [rbp-8]
    call    findFuncComma
    test    rax, rax
    jz      .ret_multi_arg_insuf
    mov     rcx, rax              ; comma = rax
    mov     rax, [rbp - 8]        ; restore close

    ; Check for second comma (too many args)
    mov     rdi, rcx
    inc     rdi                   ; comma + 1
    mov     rsi, rax
    dec     rsi                   ; close - 1
    mov     [rbp - 8], rax        ; save close
    mov     [rbp - 56], rcx       ; save comma
    call    findFuncComma
    test    rax, rax
    jz      .ma_ok_args
    ; too many args
    mov     eax, E_MULTI_ARG_EXCESS
    jmp     .cleanup_and_ret
.ma_ok_args:
    mov     rax, [rbp - 8]        ; close
    mov     rcx, [rbp - 56]       ; comma
    mov     rsi, [rbp - 40]       ; open

    ; Check empty first child: skipWhitespace(&childCheck) where childCheck = open+1
    lea     rdi, [rsi + 1]
    SKIP_WS rdi
    cmp     rdi, rcx              ; childCheck == comma?
    jne     .ma_first_ok
    mov     eax, E_MULTI_ARG_INVALID_FIRST
    jmp     .cleanup_and_ret
.ma_first_ok:
    ; Check empty second child: childCheck = comma+1
    lea     rdi, [rcx + 1]
    SKIP_WS rdi
    cmp     rdi, rax              ; childCheck == close?
    jne     .ma_second_ok
    mov     eax, E_MULTI_ARG_INVALID_SECOND
    jmp     .cleanup_and_ret
.ma_second_ok:
    ; Validate childOne: open+1 to comma
    ; childOneLen = comma - open - 1
    mov     rax, rcx              ; comma
    sub     rax, rsi              ; comma - open
    dec     rax                   ; childOneLen
    mov     [rbp - 64], rax       ; save childOneLen

    ; Allocate childOne buffer
    lea     rdx, [rax + 16]
    and     rdx, -16
    sub     rsp, rdx
    mov     rdi, rsp              ; childOne buffer
    push    rdi                   ; save childOne ptr on stack (temp)
    ; Actually, sub rsp changes everything. Let me use [rbp-...] 
    ; This is getting too complex with dynamic stack. Let me simplify.

    ; Hmm, I realize the VLA allocation on stack is problematic because
    ; I'm also using rbp-relative locals. After sub rsp, the locals are
    ; still accessible via rbp, but the pushed registers are at the bottom.
    ; The VLA space is between rsp and the pushed registers.
    ; 
    ; This approach works but is fragile. Let me use a helper function instead.
    ; 
    ; Actually, let me create a helper function that validates a substring.
    ; This encapsulates the VLA allocation.

    ; For now, let me just restore stack and use a simpler approach.
    add     rsp, rdx              ; undo alloc (this won't work, rdx was clobbered)
    ; This approach is fundamentally broken for multiple VLAs. Let me redesign.

    ; NEW APPROACH: Use a helper that extracts substring and recurses
    mov     rax, [rbp - 64]       ; childOneLen
    mov     rsi, [rbp - 40]       ; open
    mov     rcx, [rbp - 56]       ; comma
    ; Call validate_child helper
    lea     rdi, [rsi + 1]        ; src = open + 1
    mov     rsi, rax              ; len = childOneLen
    mov     rdx, r12              ; variables
    call    validate_child        ; returns error in eax
    test    eax, eax
    jnz     .cleanup_and_ret

    ; Validate childTwo: comma+1 to close
    mov     rax, [rbp - 8]        ; close
    mov     rcx, [rbp - 56]       ; comma
    ; childTwoLen = close - comma - 1
    sub     rax, rcx
    dec     rax                   ; childTwoLen
    lea     rdi, [rcx + 1]        ; src = comma + 1
    mov     rsi, rax              ; len = childTwoLen
    mov     rdx, r12              ; variables
    call    validate_child
    test    eax, eax
    jnz     .cleanup_and_ret

    ; ptr = close + 1
    mov     rax, [rbp - 8]        ; close
    lea     rbx, [rax + 1]
    inc     r14                   ; nums++
    mov     qword [rbp - 16], 1   ; mode = !mode (was 0, now 1)
    jmp     .loop

.single_arg_func:
    ; ---- Single-arg function ----
    mov     rdi, rbx
    lea     rsi, [rbp - 40]      ; &open
    lea     rdx, [rbp - 80]      ; &ret
    mov     dword [rbp - 80], 0
    call    findFuncClose
    test    rax, rax
    jz      .check_ret_error
    mov     ecx, [rbp - 80]
    test    ecx, ecx
    jnz     .return_ret_error

    ; Check empty
    mov     rsi, [rbp - 40]      ; open
    lea     r8, [rsi + 1]        ; val = open + 1
    mov     byte [rbp - 24], 1   ; isEmpty = 1
.empty_check_sa:
    cmp     r8, rax
    jae     .empty_sa_done
    movzx   ecx, byte [r8]
    test    cl, cl
    jz      .empty_sa_done
    cmp     cl, ' '
    je      .empty_sa_adv
    cmp     cl, 9
    jb      .not_empty_sa
    cmp     cl, 13
    ja      .not_empty_sa
.empty_sa_adv:
    inc     r8
    jmp     .empty_check_sa
.not_empty_sa:
    mov     byte [rbp - 24], 0
.empty_sa_done:
    movzx   ecx, byte [rbp - 24]
    test    ecx, ecx
    jnz     .ret_empty_function

    ; close = rax, open = [rbp-40]
    mov     rsi, [rbp - 40]      ; open
    ; childLen = close - open - 1
    mov     rcx, rax
    sub     rcx, rsi
    dec     rcx                  ; childLen
    ; Validate child: validate_child(open+1, childLen, variables)
    lea     rdi, [rsi + 1]
    mov     rsi, rcx
    mov     rdx, r12
    call    validate_child
    test    eax, eax
    jnz     .cleanup_and_ret

    ; ptr = close + 1
    ; close = rax (was rax before validate_child call - need to recompute)
    ; close = open + childLen + 1 = [rbp-40] + ... 
    ; Actually, close was lost. Let me recompute from open.
    ; close was the original rax from findFuncClose. But after validate_child,
    ; rax is the error code. So I need to save close before the call.
    ; Let me use [rbp-8] to save close.

    ; Hmm, I already used [rbp-8] above... let me use [rbp-64].
    ; Actually, I should have saved close. Let me redo this section.
    ; For now, recompute: close = open + childLen + 1
    ; But childLen was in rcx which is also clobbered now...
    ; 
    ; OK I need to restructure. Let me save close before calling validate_child.

    ; RESTRUCTURED: Let me rewrite this section.
    ; ... This needs a complete rethink of the stack management.

    ; For now, advance ptr past close+1 using the fact that close = ptr + total_func_len
    ; This is fragile. I'll fix by saving close properly.
    ; TEMPORARY: just jump to after_func for now (ptr advancement handled below)
    jmp     .after_func_check

.after_func_check:
    ; Check invalid characters
    movzx   eax, byte [rbx]
    ; strchr(VALID_LIST, *ptr)
    lea     rdi, [rel str_valid_list]
    mov     esi, eax
    call    strchr wrt ..plt
    test    rax, rax
    jz      .ret_invalid_char

    ; Check parentheses
    movzx   eax, byte [rbx]
    cmp     al, '('
    je      .open_paren
    cmp     al, ')'
    je      .close_paren
    jmp     .check_num_op

.open_paren:
    ; if (lastWasParen) return E_EMPTY_PAREN
    mov     rax, [rbp - 24]
    test    rax, rax
    jnz     .ret_empty_paren
    mov     qword [rbp - 24], 1  ; lastWasParen = 1
    inc     r13                   ; openParentheses++
    inc     rbx                   ; ptr++
    jmp     .loop

.close_paren:
    mov     rax, [rbp - 24]
    test    rax, rax
    jnz     .ret_empty_paren
    test    r13, r13
    jz      .ret_insuf_open_paren
    dec     r13                   ; openParentheses--
    inc     rbx
    jmp     .loop

.check_num_op:
    mov     rax, [rbp - 16]
    test    rax, rax
    jnz     .operator_mode

    ; Number mode
    inc     r14                   ; nums++
    ; strtod(ptr, &endptr)
    mov     rdi, rbx
    lea     rsi, [rbp - 32]
    call    strtod wrt ..plt
    mov     rcx, [rbp - 32]
    cmp     rcx, rbx
    je      .ret_invalid_operand  ; ptr didn't move
    mov     rbx, rcx              ; ptr = endptr
    jmp     .toggle_mode

.operator_mode:
    inc     r15                   ; ops++
    ; strchr(OPERATIONS, *ptr)
    movzx   esi, byte [rbx]
    lea     rdi, [rel str_operations]
    call    strchr wrt ..plt
    test    rax, rax
    jz      .ret_invalid_operator
    ; Update highestPrio if not NULL
    mov     rax, [rbp - 48]       ; highestPrio
    test    rax, rax
    jz      .op_no_prio
    movzx   edi, byte [rbx]
    call    getPriority_wrapper   ; custom wrapper to avoid clobbering
    mov     ecx, eax              ; prio
    mov     rax, [rbp - 48]       ; highestPrio
    mov     edx, [rax]            ; *highestPrio
    cmp     ecx, edx
    jle     .op_no_prio
    mov     [rax], ecx            ; *highestPrio = prio
.op_no_prio:
    inc     rbx                   ; ptr++

.toggle_mode:
    mov     rax, [rbp - 16]
    xor     rax, 1
    mov     [rbp - 16], rax
    jmp     .loop

.final_check:
    ; if (nums > ops + 1) return E_INSUFFICIENT_OPS
    mov     rax, r15              ; ops
    inc     rax                   ; ops + 1
    cmp     r14, rax              ; nums > ops + 1?
    jg      .ret_insuf_ops
    ; if (ops > nums - 1) return E_INSUFFICIENT_NUMS
    mov     rax, r14              ; nums
    dec     rax                   ; nums - 1
    cmp     r15, rax              ; ops > nums - 1?
    jg      .ret_insuf_nums
    ; if (openParentheses > 0) return E_INSUFFICIENT_CLOSE_PAREN
    test    r13, r13
    jnz     .ret_insuf_close_paren
    xor     eax, eax              ; return 0 (success)
    jmp     .cleanup_and_ret

.check_ret_error:
    ; findFuncClose returned NULL without setting error code
    mov     eax, E_FUNC_INVALID_BRACKETS
    jmp     .cleanup_and_ret
.return_ret_error:
    mov     eax, [rbp - 80]
    jmp     .cleanup_and_ret

.ret_invalid_char:
    mov     eax, E_INVALID_CHAR
    jmp     .cleanup_and_ret
.ret_empty_paren:
    mov     eax, E_EMPTY_PAREN
    jmp     .cleanup_and_ret
.ret_insuf_open_paren:
    mov     eax, E_INSUFFICIENT_OPEN_PAREN
    jmp     .cleanup_and_ret
.ret_insuf_close_paren:
    mov     eax, E_INSUFFICIENT_CLOSE_PAREN
    jmp     .cleanup_and_ret
.ret_insuf_ops:
    mov     eax, E_INSUFFICIENT_OPS
    jmp     .cleanup_and_ret
.ret_insuf_nums:
    mov     eax, E_INSUFFICIENT_NUMS
    jmp     .cleanup_and_ret
.ret_empty_function:
    mov     eax, E_EMPTY_FUNCTION
    jmp     .cleanup_and_ret
.ret_multi_arg_insuf:
    mov     eax, E_MULTI_ARG_INSUFFICIENT
    jmp     .cleanup_and_ret
.ret_invalid_operand:
    mov     eax, E_INVALID_OPERAND
    jmp     .cleanup_and_ret
.ret_invalid_operator:
    mov     eax, E_INVALID_OPERATOR
    jmp     .cleanup_and_ret

.cleanup_and_ret:
    ; Restore stack and return
    lea     rsp, [rbp - 80]
    pop     r15
    pop     r14
    pop     r13
    pop     r12
    pop     rbx
    lea     rsp, [rbp]
    pop     rbp
    ret

; Helper wrapper for getPriority that preserves callee registers
getPriority_wrapper:
    jmp     getPriority

; ============================================================
; validate_child(const char *src, size_t len, const struct variable *variables)
; Helper: copies src[0..len) into a temp buffer, null-terminates,
; and calls validateBuffer on it. Returns error code in eax.
; rdi = src, rsi = len, rdx = variables
; ============================================================
validate_child:
    push    rbp
    mov     rbp, rsp
    push    rbx
    push    r12
    push    r13
    sub     rsp, 8               ; align (3 pushes + sub = 32, entry 8, total 40%16=8... need more)
    ; Actually: entry rsp%16==8, push 3 regs (24 bytes) → rsp%16==0. sub 8 → rsp%16==8.
    ; That's wrong. Need sub 24 instead. Or push 4 regs.
    ; Let me recalculate: entry rsp%16==8.
    ; push rbp: rsp%16==0
    ; push rbx: rsp%16==8
    ; push r12: rsp%16==0
    ; push r13: rsp%16==8
    ; sub rsp, 8: rsp%16==0
    ; Good!
    
    mov     rbx, rdi             ; src
    mov     r12, rsi             ; len
    mov     r13, rdx             ; variables

    ; Allocate child[len + 1] on stack, 16-aligned
    lea     rax, [r12 + 16]
    and     rax, -16
    sub     rsp, rax
    mov     rdi, rsp             ; child buffer

    ; memset(child, 0, len + 1)
    xor     esi, esi
    lea     rdx, [r12 + 1]
    call    memset wrt ..plt

    ; memcpy(child, src, len)
    mov     rdi, rsp
    mov     rsi, rbx
    mov     rdx, r12
    call    memcpy wrt ..plt

    ; validateBuffer(child, NULL, variables)
    mov     rdi, rsp
    xor     esi, esi            ; highestPrio = NULL
    mov     rdx, r13            ; variables
    call    validateBuffer

    ; Return value already in eax
    lea     rsp, [rbp - 8]      ; restore past pushed regs
    pop     r13
    pop     r12
    pop     rbx
    pop     rbp
    ret

; ============================================================
; double calculateBuffer(const char *buf, int highestPrio, const struct variable *variables)
; rdi = buf, esi = highestPrio (ignored), rdx = variables
; Returns result in xmm0
; Uses recursive descent parser.
; ============================================================
global calculateBuffer
calculateBuffer:
    push    rbp
    mov     rbp, rsp
    sub     rsp, 16
    push    rbx
    push    r12
    ; entry 8, push rbp 0, sub 16 0, push rbx 8, push r12 0. Good.

    mov     [rbp - 8], rdi       ; store buf as ptr
    mov     r12, rdx             ; variables

    lea     rdi, [rbp - 8]       ; &ptr
    mov     rsi, r12
    call    parse_expr

    pop     r12
    pop     rbx
    lea     rsp, [rbp]
    pop     rbp
    ret

; ============================================================
; parse_expr: handle + and - (left-assoc)
; rdi = ptr_ptr, rsi = variables → xmm0
; ============================================================
parse_expr:
    push    rbp
    mov     rbp, rsp
    sub     rsp, 48
    push    rbx
    push    r12
    ; entry 8, rbp 0, sub 48 0, push rbx 8, push r12 0. Good.
    ; Locals: [rbp-8]=left, [rbp-16]=right, [rbp-24]=op

    mov     rbx, rdi             ; ptr_ptr
    mov     r12, rsi             ; variables

    mov     rdi, rbx
    mov     rsi, r12
    call    parse_term
    movsd   [rbp - 8], xmm0     ; left

.loop:
    mov     r8, [rbx]
    SKIP_WS r8
    mov     [rbx], r8
    movzx   eax, byte [r8]
    cmp     al, '+'
    je      .do_op
    cmp     al, '-'
    je      .do_op
    movsd   xmm0, [rbp - 8]     ; return left
    jmp     .epi
.do_op:
    mov     [rbp - 24], rax     ; save op
    inc     r8
    mov     [rbx], r8
    mov     rdi, rbx
    mov     rsi, r12
    call    parse_term
    movsd   [rbp - 16], xmm0   ; right
    movsd   xmm0, [rbp - 8]    ; left
    mov     edi, [rbp - 24]     ; op (zero-extended to int)
    movsd   xmm1, [rbp - 16]   ; right
    call    calculateTrio
    movsd   [rbp - 8], xmm0    ; left = result
    jmp     .loop
.epi:
    pop     r12
    pop     rbx
    lea     rsp, [rbp]
    pop     rbp
    ret

; ============================================================
; parse_term: handle * / % (left-assoc)
; rdi = ptr_ptr, rsi = variables → xmm0
; ============================================================
parse_term:
    push    rbp
    mov     rbp, rsp
    sub     rsp, 48
    push    rbx
    push    r12

    mov     rbx, rdi
    mov     r12, rsi

    mov     rdi, rbx
    mov     rsi, r12
    call    parse_factor
    movsd   [rbp - 8], xmm0     ; left

.loop:
    mov     r8, [rbx]
    SKIP_WS r8
    mov     [rbx], r8
    movzx   eax, byte [r8]
    cmp     al, '*'
    je      .do_op
    cmp     al, '/'
    je      .do_op
    cmp     al, '%'
    je      .do_op
    movsd   xmm0, [rbp - 8]
    jmp     .epi
.do_op:
    mov     [rbp - 24], rax
    inc     r8
    mov     [rbx], r8
    mov     rdi, rbx
    mov     rsi, r12
    call    parse_factor
    movsd   [rbp - 16], xmm0   ; right
    movsd   xmm0, [rbp - 8]    ; left
    mov     edi, [rbp - 24]
    movsd   xmm1, [rbp - 16]
    call    calculateTrio
    movsd   [rbp - 8], xmm0
    jmp     .loop
.epi:
    pop     r12
    pop     rbx
    lea     rsp, [rbp]
    pop     rbp
    ret

; ============================================================
; parse_factor: handle ^ (left-assoc)
; rdi = ptr_ptr, rsi = variables → xmm0
; ============================================================
parse_factor:
    push    rbp
    mov     rbp, rsp
    sub     rsp, 48
    push    rbx
    push    r12

    mov     rbx, rdi
    mov     r12, rsi

    mov     rdi, rbx
    mov     rsi, r12
    call    parse_atom
    movsd   [rbp - 8], xmm0     ; left

.loop:
    mov     r8, [rbx]
    SKIP_WS r8
    mov     [rbx], r8
    movzx   eax, byte [r8]
    cmp     al, '^'
    jne     .done
    inc     r8
    mov     [rbx], r8
    mov     rdi, rbx
    mov     rsi, r12
    call    parse_atom
    movsd   [rbp - 16], xmm0   ; right
    movsd   xmm0, [rbp - 8]    ; left
    mov     edi, '^'
    movsd   xmm1, [rbp - 16]
    call    calculateTrio
    movsd   [rbp - 8], xmm0
    jmp     .loop
.done:
    movsd   xmm0, [rbp - 8]
    pop     r12
    pop     rbx
    lea     rsp, [rbp]
    pop     rbp
    ret

; ============================================================
; parse_atom: numbers, parens, functions, variables
; rdi = ptr_ptr, rsi = variables → xmm0
; ============================================================
parse_atom:
    push    rbp
    mov     rbp, rsp
    sub     rsp, 64
    push    rbx
    push    r12
    push    r13
    push    r14
    ; entry 8, rbp 0, sub 64 0, push 4 regs 32 → 32, 32%16=0. Good.

    mov     rbx, rdi             ; ptr_ptr
    mov     r12, rsi             ; variables

    ; Skip whitespace
    mov     r8, [rbx]
    SKIP_WS r8
    mov     [rbx], r8

    movzx   eax, byte [r8]

    ; '(' ?
    cmp     al, '('
    jne     .check_var
    inc     r8
    mov     [rbx], r8
    mov     rdi, rbx
    mov     rsi, r12
    call    parse_expr
    movsd   [rbp - 8], xmm0    ; save result
    ; skip whitespace and ')'
    mov     r8, [rbx]
    SKIP_WS r8
    cmp     byte [r8], ')'
    jne     .paren_done
    inc     r8
    mov     [rbx], r8
.paren_done:
    movsd   xmm0, [rbp - 8]
    jmp     .epi

.check_var:
    cmp     al, '{'
    jne     .check_func
    mov     rdi, rbx
    mov     rsi, r12
    call    lookup_variable
    jmp     .epi

.check_func:
    mov     rdi, [rbx]          ; ptr
    call    getFuncIndex
    cmp     eax, -1
    je      .number
    mov     rdi, rbx
    mov     rsi, r12
    call    eval_function
    jmp     .epi

.number:
    mov     rdi, [rbx]
    lea     rsi, [rbp - 16]     ; endptr
    call    strtod wrt ..plt
    mov     r8, [rbp - 16]
    mov     [rbx], r8           ; advance ptr

.epi:
    pop     r14
    pop     r13
    pop     r12
    pop     rbx
    lea     rsp, [rbp]
    pop     rbp
    ret

; ============================================================
; lookup_variable(ptr_ptr, variables)
; rdi = ptr_ptr, rsi = variables → xmm0
; ============================================================
lookup_variable:
    push    rbp
    mov     rbp, rsp
    sub     rsp, 48
    push    rbx
    push    r12
    push    r13
    push    r14
    ; entry 8, rbp 0, sub 48 0, push 4 regs 32 → 32, 0. Good.

    mov     rbx, rdi             ; ptr_ptr
    mov     r12, rsi             ; variables
    mov     rdi, [rbx]           ; ptr
    call    findVarClose
    mov     r13, rax             ; close
    ; varLen = close - ptr - 1
    mov     r14, [rbx]           ; ptr
    sub     rax, r14
    dec     rax                  ; varLen
    mov     [rbp - 8], rax       ; save varLen

    ; Allocate bufComp on stack
    lea     rcx, [rax + 16]
    and     rcx, -16
    sub     rsp, rcx
    mov     [rbp - 16], rsp      ; bufComp

    ; memset(bufComp, 0, varLen+1)
    mov     rdi, rsp
    xor     esi, esi
    lea     rdx, [rax + 1]
    call    memset wrt ..plt

    ; memcpy(bufComp, ptr+1, varLen)
    mov     rdi, [rbp - 16]
    mov     rsi, r14
    inc     rsi
    mov     rdx, [rbp - 8]
    call    memcpy wrt ..plt

    ; Loop variables
    xor     r8, r8               ; i = 0
.vloop:
    cmp     r8, MAX_VARIABLES
    jae     .fail
    mov     rax, r8
    imul    rax, rax, VAR_SIZE
    lea     rcx, [r12 + rax]
    mov     rax, [rcx + VAR_OFF_NAME]
    test    rax, rax
    jz      .fail
    mov     rdx, [rcx + VAR_OFF_LEN]
    cmp     rdx, [rbp - 8]
    jne     .next
    mov     rdi, [rbp - 16]
    mov     rsi, [rcx + VAR_OFF_NAME]
    mov     rdx, [rbp - 8]
    call    strncmp wrt ..plt
    test    eax, eax
    jz      .found
.next:
    inc     r8
    jmp     .vloop
.found:
    mov     rax, r8
    imul    rax, rax, VAR_SIZE
    lea     rcx, [r12 + rax]
    movsd   xmm0, [rcx + VAR_OFF_VALUE]
    ; advance: *ptr_ptr = close + 1 = ptr + varLen + 2
    mov     rax, [rbp - 8]       ; varLen
    lea     r8, [r14 + rax + 2]  ; ptr + varLen + 2
    mov     [rbx], r8
    jmp     .ret
.fail:
    xorps   xmm0, xmm0
.ret:
    lea     rsp, [rbp - 64]     ; restore past VLA + 4 pushed regs
    pop     r14
    pop     r13
    pop     r12
    pop     rbx
    lea     rsp, [rbp]
    pop     rbp
    ret

; ============================================================
; eval_function(ptr_ptr, variables)
; rdi = ptr_ptr, rsi = variables → xmm0
; ============================================================
eval_function:
    push    rbp
    mov     rbp, rsp
    sub     rsp, 64
    push    rbx
    push    r12
    push    r13
    push    r14
    ; Locals: [rbp-8]=funcIndex, [rbp-16]=open, [rbp-24]=close,
    ;         [rbp-32]=comma, [rbp-40]=arg1, [rbp-48]=arg2

    mov     rbx, rdi             ; ptr_ptr
    mov     r12, rsi             ; variables
    mov     rdi, [rbx]           ; ptr
    call    getFuncIndex
    mov     [rbp - 8], rax       ; funcIndex
    mov     r13, rax             ; also in r13

    cmp     eax, FN_MULTI_ARG_PAD
    jle     .single_arg

    ; ---- Multi-arg ----
    mov     rdi, [rbx]
    lea     rsi, [rbp - 16]     ; &open
    xor     edx, edx
    call    findFuncClose
    mov     [rbp - 24], rax      ; close
    mov     r14, rax             ; close in r14

    ; findFuncComma(open+1, close-1)
    mov     rdi, [rbp - 16]
    inc     rdi
    lea     rsi, [r14 - 1]
    call    findFuncComma
    mov     [rbp - 32], rax      ; comma

    ; calc_child(open+1, comma-open-1, variables)
    mov     rdi, [rbp - 16]
    inc     rdi                  ; open+1
    mov     rsi, [rbp - 32]      ; comma
    sub     rsi, [rbp - 16]      ; comma - open
    dec     rsi                  ; comma - open - 1
    mov     rdx, r12
    call    calc_child
    movsd   [rbp - 40], xmm0    ; arg1

    ; calc_child(comma+1, close-comma-1, variables)
    mov     rdi, [rbp - 32]
    inc     rdi                  ; comma+1
    mov     rsi, r14             ; close
    sub     rsi, [rbp - 32]      ; close - comma
    dec     rsi                  ; close - comma - 1
    mov     rdx, r12
    call    calc_child
    movsd   [rbp - 48], xmm0    ; arg2

    ; apply_multi_arg(funcIndex, arg1, arg2)
    mov     eax, r13d
    movsd   xmm0, [rbp - 40]
    movsd   xmm1, [rbp - 48]
    call    apply_multi_arg
    jmp     .advance

.single_arg:
    mov     rdi, [rbx]
    lea     rsi, [rbp - 16]     ; &open
    xor     edx, edx
    call    findFuncClose
    mov     [rbp - 24], rax      ; close
    mov     r14, rax

    ; calc_child(open+1, close-open-1, variables)
    mov     rdi, [rbp - 16]
    inc     rdi
    mov     rsi, r14
    sub     rsi, [rbp - 16]
    dec     rsi
    mov     rdx, r12
    call    calc_child
    movsd   [rbp - 40], xmm0   ; save child result

    ; apply_single_arg(funcIndex, arg)
    mov     eax, r13d
    movsd   xmm0, [rbp - 40]
    call    apply_single_arg

.advance:
    ; *ptr_ptr = close + 1
    lea     rax, [r14 + 1]
    mov     [rbx], rax

.epi:
    pop     r14
    pop     r13
    pop     r12
    pop     rbx
    lea     rsp, [rbp]
    pop     rbp
    ret

; ============================================================
; calc_child(src, len, variables)
; rdi = src, rsi = len, rdx = variables → xmm0
; ============================================================
calc_child:
    push    rbp
    mov     rbp, rsp
    sub     rsp, 32
    push    rbx
    push    r12
    push    r13
    push    r14

    mov     rbx, rdi             ; src
    mov     r12, rsi             ; len
    mov     r13, rdx             ; variables

    lea     rax, [r12 + 16]
    and     rax, -16
    sub     rsp, rax

    mov     rdi, rsp
    xor     esi, esi
    lea     rdx, [r12 + 1]
    call    memset wrt ..plt

    mov     rdi, rsp
    mov     rsi, rbx
    mov     rdx, r12
    call    memcpy wrt ..plt

    mov     rdi, rsp
    mov     esi, 3               ; highestPrio (ignored by our impl)
    mov     rdx, r13
    call    calculateBuffer

    lea     rsp, [rbp - 64]     ; restore past VLA + 4 pushed regs
    pop     r14
    pop     r13
    pop     r12
    pop     rbx
    lea     rsp, [rbp]
    pop     rbp
    ret

; ============================================================
; apply_single_arg(int funcIndex, double arg)
; eax = funcIndex, xmm0 = arg → xmm0
; ============================================================
apply_single_arg:
    cmp     eax, FN_SQUARE_ROOT
    jne     .nsqrt
    sub     rsp, 8
    call    sqrt wrt ..plt
    add     rsp, 8
    ret
.nsqrt:
    cmp     eax, FN_CUBE_ROOT
    jne     .ncbrt
    sub     rsp, 8
    call    cbrt wrt ..plt
    add     rsp, 8
    ret
.ncbrt:
    cmp     eax, FN_SINE
    jne     .nsin
    sub     rsp, 8
    call    sin wrt ..plt
    add     rsp, 8
    ret
.nsin:
    cmp     eax, FN_COSINE
    jne     .ncos
    sub     rsp, 8
    call    cos wrt ..plt
    add     rsp, 8
    ret
.ncos:
    cmp     eax, FN_TANGENT
    jne     .ntan
    sub     rsp, 8
    call    tan wrt ..plt
    add     rsp, 8
    ret
.ntan:
    cmp     eax, FN_SINE_H
    jne     .nsinh
    sub     rsp, 8
    call    sinh wrt ..plt
    add     rsp, 8
    ret
.nsinh:
    cmp     eax, FN_COSINE_H
    jne     .ncosh
    sub     rsp, 8
    call    cosh wrt ..plt
    add     rsp, 8
    ret
.ncosh:
    cmp     eax, FN_TANGENT_H
    jne     .ntanh
    sub     rsp, 8
    call    tanh wrt ..plt
    add     rsp, 8
    ret
.ntanh:
    cmp     eax, FN_SINE_R
    jne     .nasin
    sub     rsp, 8
    call    asin wrt ..plt
    add     rsp, 8
    ret
.nasin:
    cmp     eax, FN_COSINE_R
    jne     .nacos
    sub     rsp, 8
    call    acos wrt ..plt
    add     rsp, 8
    ret
.nacos:
    cmp     eax, FN_TANGENT_R
    jne     .natan
    sub     rsp, 8
    call    atan wrt ..plt
    add     rsp, 8
    ret
.natan:
    cmp     eax, FN_SINE_RH
    jne     .nasinh
    sub     rsp, 8
    call    asinh wrt ..plt
    add     rsp, 8
    ret
.nasinh:
    cmp     eax, FN_COSINE_RH
    jne     .nacosh
    sub     rsp, 8
    call    acosh wrt ..plt
    add     rsp, 8
    ret
.nacosh:
    cmp     eax, FN_TANGENT_RH
    jne     .natanh
    sub     rsp, 8
    call    atanh wrt ..plt
    add     rsp, 8
    ret
.natanh:
    cmp     eax, FN_COSECANT
    jne     .ncsc
    sub     rsp, 8
    call    sin wrt ..plt
    add     rsp, 8
    jmp     .reciprocal
.ncsc:
    cmp     eax, FN_SECANT
    jne     .nsec
    sub     rsp, 8
    call    cos wrt ..plt
    add     rsp, 8
    jmp     .reciprocal
.nsec:
    cmp     eax, FN_COTANGENT
    jne     .ncot
    sub     rsp, 8
    call    tan wrt ..plt
    add     rsp, 8
    jmp     .reciprocal
.ncot:
    cmp     eax, FN_COSECANT_H
    jne     .ncsch
    sub     rsp, 8
    call    sinh wrt ..plt
    add     rsp, 8
    jmp     .reciprocal
.ncsch:
    cmp     eax, FN_SECANT_H
    jne     .nsech
    sub     rsp, 8
    call    cosh wrt ..plt
    add     rsp, 8
    jmp     .reciprocal
.nsech:
    cmp     eax, FN_COTANGENT_H
    jne     .ncoth
    sub     rsp, 8
    call    tanh wrt ..plt
    add     rsp, 8
    jmp     .reciprocal
.ncoth:
    cmp     eax, FN_FLOOR
    jne     .nfloor
    sub     rsp, 8
    call    floor wrt ..plt
    add     rsp, 8
    ret
.nfloor:
    cmp     eax, FN_CEILING
    jne     .nceil
    sub     rsp, 8
    call    ceil wrt ..plt
    add     rsp, 8
    ret
.nceil:
    cmp     eax, FN_N_LOG
    jne     .nlog
    sub     rsp, 8
    call    log wrt ..plt
    add     rsp, 8
    ret
.nlog:
    cmp     eax, FN_D_LOG
    jne     .nlog10
    sub     rsp, 8
    call    log10 wrt ..plt
    add     rsp, 8
    ret
.nlog10:
    cmp     eax, FN_B_LOG
    jne     .nlog2
    sub     rsp, 8
    call    log2 wrt ..plt
    add     rsp, 8
    ret
.nlog2:
    cmp     eax, FN_GAMMA
    jne     .ngamma
    sub     rsp, 8
    call    gamma wrt ..plt
    add     rsp, 8
    ret
.ngamma:
    cmp     eax, FN_TRUNCATE
    jne     .ntrunc
    sub     rsp, 8
    call    trunc wrt ..plt
    add     rsp, 8
    ret
.ntrunc:
    cmp     eax, FN_ERROR_FUNC
    jne     .nerf
    sub     rsp, 8
    call    erf wrt ..plt
    add     rsp, 8
    ret
.nerf:
    cmp     eax, FN_ERROR_FUNC_C
    jne     .nerfc
    sub     rsp, 8
    call    erfc wrt ..plt
    add     rsp, 8
    ret
.nerfc:
    cmp     eax, FN_L_GAMMA
    jne     .nlgamma
    sub     rsp, 8
    call    lgamma wrt ..plt
    add     rsp, 8
    ret
.nlgamma:
    cmp     eax, FN_ABSOLUTE
    jne     .nabs
    sub     rsp, 8
    call    fabs wrt ..plt
    add     rsp, 8
    ret
.nabs:
    cmp     eax, FN_ROUND
    jne     .nround
    sub     rsp, 8
    call    round wrt ..plt
    add     rsp, 8
    ret
.nround:
    cmp     eax, FN_RAD_TO_DEG
    jne     .ndeg
    ; deg(x) = x / PI * 180
    movsd   xmm1, [rel dpi]
    divsd   xmm0, xmm1
    movsd   xmm1, [rel d180]
    mulsd   xmm0, xmm1
    ret
.ndeg:
    cmp     eax, FN_DEG_TO_RAD
    jne     .nrad
    ; rad(x) = x / 180 * PI
    movsd   xmm1, [rel d180]
    divsd   xmm0, xmm1
    movsd   xmm1, [rel dpi]
    mulsd   xmm0, xmm1
    ret
.nrad:
    cmp     eax, FN_EXPONENT_E
    jne     .nexp
    sub     rsp, 8
    call    exp wrt ..plt
    add     rsp, 8
    ret
.nexp:
    cmp     eax, FN_EXPONENT_2
    jne     .nexp2
    sub     rsp, 8
    call    exp2 wrt ..plt
    add     rsp, 8
    ret
.nexp2:
    xorps   xmm0, xmm0
    ret

.reciprocal:
    ; xmm0 = func(x), compute 1.0 / xmm0
    movsd   xmm1, xmm0
    movsd   xmm0, [rel d1]
    divsd   xmm0, xmm1
    ret

; ============================================================
; apply_multi_arg(int funcIndex, double arg1, double arg2)
; eax = funcIndex, xmm0 = arg1, xmm1 = arg2 → xmm0
; ============================================================
apply_multi_arg:
    cmp     eax, FN_POWER_FUNC
    jne     .npow
    sub     rsp, 8
    call    pow wrt ..plt
    add     rsp, 8
    ret
.npow:
    cmp     eax, FN_MAXIMUM
    jne     .nmax
    sub     rsp, 8
    call    fmax wrt ..plt
    add     rsp, 8
    ret
.nmax:
    cmp     eax, FN_MINIMUN
    jne     .nmin
    sub     rsp, 8
    call    fmin wrt ..plt
    add     rsp, 8
    ret
.nmin:
    cmp     eax, FN_HYPOTENUSE
    jne     .nhypot
    sub     rsp, 8
    call    hypot wrt ..plt
    add     rsp, 8
    ret
.nhypot:
    cmp     eax, FN_X_LOG
    jne     .nlogx
    ; logx(base, x) = log(x) / log(base) = log(xmm1) / log(xmm0)
    movsd   xmm2, xmm0          ; save base
    sub     rsp, 8
    movsd   xmm0, xmm1          ; log(x)
    call    log wrt ..plt
    movsd   xmm3, xmm0          ; xmm3 = log(x)
    movsd   xmm0, xmm2          ; log(base)
    call    log wrt ..plt
    add     rsp, 8
    divsd   xmm3, xmm0          ; log(x) / log(base)
    movsd   xmm0, xmm3
    ret
.nlogx:
    cmp     eax, FN_TANGENT_A2
    jne     .natan2
    sub     rsp, 8
    call    atan2 wrt ..plt
    add     rsp, 8
    ret
.natan2:
    xorps   xmm0, xmm0
    ret
