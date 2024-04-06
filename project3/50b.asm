bits    64

default rel

global  main

extern  scanf
extern  printf

section .data
    series1  dq 1.0
    series2  dq 1.0
    num1     dq 1.0
    num2     dq 1.0
    den      dq 1.0
    format   db '%d %lf', 0
    fout1    db "e^x = %lf ", 0xA, 0
    fout2    db "e^(x+1) = %lf ", 0xA, 0
    count    dd 1

section .bss
    k       resd 1 ;int k
    x       resq 1 ;double x
    xp      resq 1 ;double x + 1

section .text
main:
    sub     rsp, 8

    lea     rdx, [x]
    lea     rsi, [k]
    lea     rdi, [format]
    mov     al, 0
    call    scanf wrt ..plt

    movlpd  xmm0, qword [x]
    addsd   xmm0, qword [num1]
    movlpd  qword [xp], xmm0

maclaurin:
    movlpd  xmm0, qword [num1]
    movhpd  xmm0, qword [num2]
    movlpd  xmm1, qword [x]
    movhpd  xmm1, qword [xp]
    mulpd   xmm0, xmm1
    movlpd  qword [num1], xmm0
    movhpd  qword [num2], xmm0

    movlpd  xmm0, qword [den]
    cvtsi2sd xmm1, dword [count]
    mulsd   xmm0, xmm1
    movlpd  qword [den], xmm0

    movlpd  xmm0, qword [num1]
    movhpd  xmm0, qword [num2]
    movlpd  xmm1, qword [den]
    movhpd  xmm1, qword [den]
    divpd   xmm0, xmm1
    movlpd  xmm1, qword [series1]
    movhpd  xmm1, qword [series2]
    addpd   xmm1, xmm0
    movlpd  qword [series1], xmm1
    movhpd  qword [series2], xmm1
    
    add     dword [count], 1
    mov     eax, dword [count]
    cmp     eax, dword [k]
    jle     maclaurin

print:
    movlpd  xmm0, qword [series1]
    lea     rdi, [fout1]
    mov     al, 1
    call    printf wrt ..plt
    
    movlpd  xmm0, qword [series2]
    lea     rdi, [fout2]
    mov     al, 1
    call    printf wrt ..plt

    add     rsp, 8
    sub     rax, rax
    ret