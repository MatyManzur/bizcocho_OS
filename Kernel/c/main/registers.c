// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include <registers.h>

#define REGISTER_COUNT 18
#define HALF 0x100000000

static registers_t registers = {0};

static void printRegAux(uint64_t *registers, uint8_t index, char *registerNames[]);

void saveRegisters()
{
    getRegisters(&(registers.rax));
}

void getLastRegisters(struct registers_t *registersCopy)
{
    registersCopy->rax = registers.rax;
    registersCopy->rbx = registers.rbx;
    registersCopy->rcx = registers.rcx;
    registersCopy->rdx = registers.rdx;
    registersCopy->rsi = registers.rsi;
    registersCopy->rdi = registers.rdi;
    registersCopy->r8 = registers.r8;
    registersCopy->r9 = registers.r9;
    registersCopy->r10 = registers.r10;
    registersCopy->r11 = registers.r11;
    registersCopy->r12 = registers.r12;
    registersCopy->r13 = registers.r13;
    registersCopy->r14 = registers.r14;
    registersCopy->r15 = registers.r15;
    registersCopy->rip = registers.rip;
    registersCopy->rbp = registers.rbp;
    registersCopy->rsp = registers.rsp;
    registersCopy->flags = registers.flags;
}

static void printRegAux(uint64_t *registers, uint8_t index, char *registerNames[])
{
    write(STDERR, registerNames[index]);
    write(STDERR, "=  0x");
    write(STDERR, convert(registers[index] / HALF, 16, 8));
    write(STDERR, convert(registers[index] % HALF, 16, 8));
    write(STDERR, "\n");
}

void printRegisters(uint64_t *registers)
{

    char *registerNames[] = {"eflags  ", "rsp  ", "rip  ", "r15  ", "r14  ", "r13  ", "r12  ", "r11  ",
                             "r10  ", "r9  ", "r8  ", "rsi  ", "rdi  ", "rbp  ", "rdx  ", "rcx  ", "rbx  ",
                             "rax  "};

    for (int i = REGISTER_COUNT - 1; i >= 0; i--)
    {
        printRegAux(registers, i, registerNames);
    }
}
