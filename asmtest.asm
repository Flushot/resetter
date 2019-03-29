bits 64

; nasm function macros
%macro prolog 0
    push rbp     ; save ebp of caller
    mov rbp, rsp ; save start of stack frame
%endmacro
%macro prolog 1
    prolog
    sub rsp, %1  ; allocate locals
%endmacro
%macro epilog 0
    mov rsp, rbp ; deallocate locals
    pop rbp      ; restore ebp to caller location
    ret          ; return to caller
%endmacro

segment .data
    msg db "testing 123", 0xd, 0xa
    msg_len equ $ - msg

segment .text
global _testfunc
_testfunc:
    prolog
    ; cdecl
    push rax
    push rcx
    push rdx

    ; print
    mov rax, 0x2000004 ; write syscall (mac)
    mov rdi, 1 ; stdout
    lea rsi, [rel msg] ; message
    mov rdx, msg_len ; len
    syscall

    ; cdecl
    pop rdx
    pop rcx
    pop rax
    epilog
