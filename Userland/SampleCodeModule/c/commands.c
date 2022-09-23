
#include <commands.h>

#define HALF 0x100000000
#define REGISTER_COUNT 18

// imprime el valor de los registros al momento de llamarse
static void printRegAux(uint64_t *registers, uint8_t index, char *registerNames[]);


static void printRegAux(uint64_t *registers, uint8_t index, char *registerNames[])
{
    printStringColor(registerNames[index], BLUE, WHITE);
    printWithFormat(" =  0x%8x%8x", registers[index] / HALF, registers[index] % HALF);
    newLine();
}

void printRegisters(uint8_t argc, void **argv)
{

    registers_t registers = {0};

    sys_get_last_registers(&registers);

    char *registerNames[] = {"rax  ", "rbx  ", "rcx  ", "rdx  ", "rsi  ", "rdi  ", "r8   ", "r9   ", "r10  ",
                             "r11  ", "r12  ", "r13  ", "r14  ", "r15  ", "rip  ", "rbp  ", "rsp  ", "flags"};

    for (uint8_t i = 0; i < REGISTER_COUNT; i++)
    {
        printRegAux((uint64_t * ) & registers, i, registerNames);
    }

    sys_exit();
}

// imprime los distintos comandos a los que se puede acceder
void help(uint8_t argc, void **argv)
{
    printStringColor(" \2 \2 \2 \2 \2 Bienvenidos a Bizcocho! \2 \2 \2 \2 \2 ", YELLOW, BLACK);
    sys_new_line(YELLOW);
    printStringColor("-----TECLAS-----  ", YELLOW, BLACK);
    printStringColor("(en rojo unicamente para \"modo pipe\")", YELLOW, RED);
    sys_new_line(YELLOW);
    printStringColor("ESC: mata al programa | N: pausa/reanudar programa principal", YELLOW, BLACK);
    printStringColor("/izquierdo", YELLOW, RED);
    sys_new_line(YELLOW);
    printStringColor("F1: realiza snapshot de registros ", YELLOW, BLACK);
    printStringColor("| M: pausa/reanudar programa derecho", YELLOW, RED);
    sys_new_line(YELLOW);
    printStringColor("J: mata programa izquierdo | K: mata programa derecho", YELLOW, RED);
    sys_new_line(YELLOW);
    printStringColor("Los comandos disponibles son los siguientes: ", WHITE, MAGENTA);
    sys_new_line(WHITE);
    printString("help: Despliega los comandos y programas accesibles.");
    newLine();
    printString("time: Despliega el dia y la hora.");
    newLine();
    printString("inforeg: Despliega el valor de los registros del ultimo snapshot");
    newLine();
    printString("prime: Imprime todos los numeros primos uno atras del otro.");
    newLine();
    printString("fibonacci: Imprime la serie de Fibonacci.");
    newLine();
    printString("clear: Borra todo el contenido de la pantalla.");
    newLine();
    printString("divZero: Intenta dividir por cero.");
    newLine();
    printString(
            "printmem \'hexa\': Imprime la zona de memoria elegida por el usuario. Debe ser indicada en hexadecimal sin h al final.");
    newLine();
    printString("invalidOpcode: Intenta realizar un procedimiento no permitido.");
    newLine();
    printString("\'programa1\' | \'programa2\': (MODO PIPE) Corre ambos programas simult");
    putChar(160);
    printString("neamente.");
    newLine();
    printString("letter \'num\': Cambia el color de las letras de esta terminal.");
    newLine();
    printString("background \'num\': Cambia el color del fondo de esta terminal.");
    newLine();
    printString("user \'num\': Cambia el color del user de esta terminal.");
    newLine();
    printString("boquita: Cambia a los colores xeneizes. (river y banfield sirven)");
    newLine();
    printString("monkey: Despliega 4 monitos de lo m");
    putChar(160);
    printString("s simp");
    putChar(160);
    printString("ticos.");
    newLine();
    sys_exit();
}

// imprime en pantalla la fecha y el horario (GMT-3)
void time(uint8_t argc, void **argv)
{
    struct datetime_t data;
    struct timezone_t tzone = {-3, 0};
    sys_set_time_zone(&tzone);
    sys_get_current_date_time(&data, &tzone);
    printWithFormat("%2d:%2d:%2d", data.hours, data.mins, data.secs);
    putChar(' ');
    printWithFormat("%2d/%2d/%2d", data.day, data.month, data.year);
    newLine();
    sys_exit();
}

// imprime la zona de memoria escogida por el usuario
void printmem(uint8_t argc, void **argv)
{
    if (argc != 1)
    {
        printString("printmem needs exactly one argument");
        sys_exit();
    }
    char *argument = (char *) argv[0];
    uint64_t address;
    int errorFlag = xtou64(argument, &address);
    if (errorFlag)
    {
        printString(argument);
        printString(" is not a valid memory address");
        newLine();
        sys_exit();
    }
    uint8_t buffer[32] = {0};

    int error = sys_memory_dump(address, buffer);
    if (error == -1)
    {
        printString(argument);
        printString(" is out of bounds");
        newLine();
        sys_exit();
    }
    printWithFormat("The memory dump from the following address 0x%8x%8x", address / HALF, address % HALF);

    newLine();
    for (int i = 0; i < 32; i++)
    {
        printWithFormat("0x%2x", buffer[i]);
        putChar(' ');
        if ((i + 1) % 8 == 0)
            newLine();
    }

    sys_exit();
}

// una función cuyo objetivo es romperse. Divide por cero sin temor alguno
void divZero(uint8_t argc, void **argv)
{
    int a = 8;
    int b = 1;
    b--;
    int c = a / b;
}

