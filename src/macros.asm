; macros.asm - Common NASM macros for SysV AMD64 ABI
; All functions follow the System V AMD64 calling convention

; Standard prologue. %1 = local stack frame size (must be 16-aligned for calls)
%macro PROLOGUE 1
    push    rbp
    mov     rbp, rsp
    sub     rsp, %1
%endmacro

; Standard epilogue
%macro EPILOGUE 0
    lea     rsp, [rbp]
    pop     rbp
    ret
%endmacro

; Call a libc function. Saves the 2 return values back into rdx after the call.
; Assumes arguments are already set per ABI.
%macro CALL_LIBC 1
    call    %1
%endmacro

; Compute strlen-style length of a null-terminated string in %1, result in rax.
; Clobbers rax, rcx, rdi (if %1 is rdi).
%macro STRLEN 2   ; %1 = src reg, %2 = dst reg
    mov     %2, %1
%%loop:
    cmp     byte [%2], 0
    je      %%done
    inc     %2
    jmp     %%loop
%%done:
    sub     %2, %1
%endmacro

; Allocate a VLA-like stack array of %1 bytes, putting the resulting pointer in %2.
; Must be balanced by VLA_FREE.
%macro VLA_ALLOC 2  ; %1 = size reg, %2 = dest ptr reg
    mov     %2, rsp
    sub     %2, %1
    and     %2, -16          ; 16-byte align
    mov     rsp, %2
%endmacro

; ---- Inline skipWhitespace ----
; Advances pointer in %1 past leading whitespace. Clobbers eax.
%macro SKIP_WS 1
%%ws_loop:
    movzx   eax, byte [%1]
    test    al, al
    jz      %%ws_done
    cmp     al, ' '
    je      %%ws_adv
    cmp     al, 9
    jb      %%ws_done
    cmp     al, 13
    ja      %%ws_done
%%ws_adv:
    inc     %1
    jmp     %%ws_loop
%%ws_done:
%endmacro

; ---- Allocate aligned stack space for a child buffer ----
; %1 = size (register), %2 = dest pointer register
; Allocates size+1 (for null terminator), 16-byte aligned
%macro ALLOC_CHILD 2
    lea     %2, [%1 + 16]        ; size + 1 + padding
    and     %2, -16
    sub     rsp, %2
%endmacro
