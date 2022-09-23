#include <registers.h>

#define REGISTER_COUNT 18
#define HALF 0x100000000

static registers_t registers = {0};

static void printRegAux(uint64_t *registers, uint8_t index, format_t *registerformat, format_t *numberformat,
                        char *registerNames[]);

void saveRegisters()
{
    getRegisters(&registers);
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

static void printRegAux(uint64_t *registers, uint8_t index, format_t *registerformat, format_t *numberformat,
                        char *registerNames[])
{
    print(registerNames[index], registerformat);
    print("=  0x", numberformat);
    print(convert(registers[index] / HALF, 16, 8), numberformat);
    print(convert(registers[index] % HALF, 16, 8), numberformat);
    newLine(BLACK);
}


void printRegisters(uint64_t *registers)
{
    struct format_t registerformat = {BLACK, YELLOW};
    struct format_t numberformat = {BLACK, WHITE};

    char *registerNames[] = {"eflags  ", "rsp  ", "rip  ", "r15  ", "r14  ", "r13  ", "r12  ", "r11  ",
                                      "r10  ", "r9  ", "r8  ", "rsi  ", "rdi  ", "rbp  ", "rdx  ", "rcx  ", "rbx  ",
                                      "rax  "};

    for (int i = REGISTER_COUNT - 1; i >= 0; i--)
    {
        printRegAux(registers, i, &registerformat, &numberformat, registerNames);
    }
}
