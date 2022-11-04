// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include <lib.h>
#include <registers.h>
#include <files.h>

#define ZERO_EXCEPTION_ID 0
#define INVALID_OPCODE_ID 6

static void zero_division();

static void invalid_opcode();

void exceptionDispatcher(int exception, uint64_t *registers) // llamado desde interrupts.asm
{
    if (exception == ZERO_EXCEPTION_ID)
        zero_division(registers);
    if (exception == INVALID_OPCODE_ID)
        invalid_opcode(registers);
}

static void zero_division(uint64_t *registers)
{
    char *errorMsg = "An error occurred: division by zero exception! \n";
    write(STDERR, errorMsg);
    printRegisters(registers);

    exit(100 + ZERO_EXCEPTION_ID);
}

static void invalid_opcode(uint64_t *registers)
{
    char *errorMsg = "An error occurred: invalid opcode exception! \n";
    write(STDERR, errorMsg);
    printRegisters(registers);

    exit(100 + INVALID_OPCODE_ID);
}
