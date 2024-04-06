bits    64

default rel

global  main

extern  scanf
extern  printf

section .data
    c1      dq 0.0
    c2      dq 0.125
    format  db '%lf', 0
    fout    db "sqrt(%lf) = %lf", 0xA, 0

section .bss
    end    resq 1
    d      resq 1    

section .text
main:
    sub     rsp, 8
    
    lea     rsi, [end]
    lea     rdi, [format]
    mov     al, 0
    call    scanf wrt ..plt

    movlpd  xmm0, [c1]
    movlpd  qword [d], xmm0

repeat:
    movlpd  xmm0, qword [d]
    sqrtsd  xmm1, [d]
    lea     rdi, [fout]
    mov     al, 2
    call    printf wrt ..plt

    movlpd  xmm0, qword [c2]
    movlpd  xmm1, qword [d]
    addsd   xmm1, xmm0
    
    movlpd  qword [d], xmm1
    movlpd  xmm0, qword [d]
    cmpltsd xmm0, qword [end]

    movq    rax, xmm0
    cmp     rax, 0
    jl      repeat


add     rsp, 8
sub     rax, rax
ret