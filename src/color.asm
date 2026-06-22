; color.asm - Syntax highlighting (printBufColored)

%include "macros.asm"
%include "equates.inc"

extern printf
extern getFuncIndex
extern findFuncClose
extern findVarClose
extern strtod
extern memset

SECTION .text

; ============================================================
; void printBufColored(const char *buf)
; rdi = buf
; ============================================================
global printBufColored
printBufColored:
    ; For now, just print the buffer without colors
    ; This is a stub that will be replaced
    push    rbp
    mov     rbp, rsp
    sub     rsp, 8              ; align
    mov     rsi, rdi            ; buf
    lea     rdi, [rel .fmt]
    xor     eax, eax
    call    printf wrt ..plt
    add     rsp, 8
    pop     rbp
    ret

SECTION .rodata
.fmt: db "%s", 0
