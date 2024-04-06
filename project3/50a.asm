bits    64

default rel

global  main

extern  scanf
extern  printf

section .data
    series  dq 1.0
    num     dq 1.0
    den     dq 1.0
    format  db '%d %lf', 0
    fout    db "e^x = %lf ", 0xA, 0
    count   dd 1

section .bss
    k       resd 1 ;int k
    x       resq 1 ;double x

section .text
main:
    sub     rsp, 8

    lea     rdx, [x]
    lea     rsi, [k]
    lea     rdi, [format]
    mov     al, 0
    call    scanf wrt ..plt

maclaurin:
    movlpd  xmm0, qword [num]
    movhpd  xmm0, qword [den]
    
    movlpd  xmm1, qword [x]
    cvtsi2sd xmm2, dword [count]
    movlpd  qword [den], xmm2
    movhpd  xmm1, qword [den]

    mulpd   xmm0, xmm1

    movlpd  qword [num], xmm0
    movhpd  qword [den], xmm0
    movlpd  xmm1, qword [den]
    divsd   xmm0, xmm1
    
    movlpd  xmm1, qword [series]
    addsd   xmm1, xmm0
    movlpd  qword [series], xmm1

    add     dword [count], 1
    mov     eax, dword [count]
    cmp     eax, dword [k]
    jle     maclaurin

print:
    movlpd  xmm0, qword [series]
    lea     rdi, [fout]
    mov     al, 1
    call    printf wrt ..plt

    add     rsp, 8
    sub     rax, rax
    ret