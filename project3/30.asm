bits    64

default rel

global  main

extern  scanf
extern  printf

section .data
    format_input  db '%s', 0
    format_output  db '%s', 0xA, 0

section .bss
    input   resb 1024
    output  resb 1024

section .text
main:
    sub     rsp, 8

    lea     rdi, [format_input]
    lea     rsi, [input]
    mov     al, 0
    call    scanf wrt ..plt

    lea     rsi, [input]
    lea     rdi, [output]
    mov     rcx, 1024
    rep     movsb

    lea     rdi, [format_output]
    lea     rsi, [output]
    mov     al, 0
    call    printf wrt ..plt

    add     rsp, 8
    sub     rax, rax
    ret