bits    64

default rel

global  main

extern  scanf
extern  printf

section .data
    format  db '%d ', 0

section .bss
    array   resd 100
    size    resd 1
    count   resd 1 ;trzeba później wyzerować
    count2  resd 1
    tmp     resd 1

section .text
main:
    sub     rsp, 8

read_loop:
    lea     rsi, [array]
    add     rsi, [size]

    lea     rdi, [format]
    mov     al, 0
    call    scanf wrt ..plt
    cmp     rax, 1
    jne     setup

    add     dword [size], 4 ;dword - pass value, no dword - pass address
    jmp     read_loop

setup:
    mov     dword [count], -4

bubble:
    add     dword [count], 4
    mov     ecx, dword [count]
    cmp     ecx, dword [size]
    jge     print_loop
    mov     ecx, dword [size]
    mov     dword [count2], ecx

inloop:
    sub     dword [count2], 4
    mov     ecx, dword [count2]
    cmp     ecx, dword [count]
    jle     bubble
    
    lea     rdi, [array]
    add     rdi, [count2]

    lea     rsi, [array]
    add     rsi, [count2]
    sub     rsi, 4

    mov     ecx, dword [rdi]

    cmp     ecx, dword [rsi]
    jl      swap
    jmp     inloop
    
swap:
    xchg    ecx, dword [rsi]
    xchg    dword [rdi], ecx
    jmp     inloop

print_loop:
    lea     rax, [array]
    add     rax, [tmp]
    mov     esi, dword [rax] ;esi instead of rsi in order to match dword size

    lea     rdi, [format]
    mov     al, 0
    call    printf wrt ..plt

    add     dword [tmp], 4
    mov     edi, dword [tmp]
    mov     esi, dword [size]
    cmp     edi, esi
    jl      print_loop

terminate:
    add     rsp, 8
    sub     rax, rax
    ret