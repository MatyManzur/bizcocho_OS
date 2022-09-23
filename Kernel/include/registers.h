#ifndef REGISTERS_H
#define REGISTERS_H

#include <lib.h>
#include <printing.h>

typedef struct registers_t
{
    uint64_t rax;
    uint64_t rbx;
    uint64_t rcx;
    uint64_t rdx;
    uint64_t rsi;
    uint64_t rdi;
    uint64_t r8;
    uint64_t r9;
    uint64_t r10;
    uint64_t r11;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;
    uint64_t rip;
    uint64_t rbp;
    uint64_t rsp;
    uint64_t flags;
} registers_t;

void getRegisters(uint64_t *registers); //en ASM

void saveRegisters(); //para que lo llame el keyboard handler
void printRegisters(); //para que lo llamen las exceptions

//syscalls

void getLastRegisters(struct registers_t *registers);


#endif
