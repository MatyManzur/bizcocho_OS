#include <fibonacci.h>

uint64_t printNewValue(uint64_t value1, uint64_t value2, uint8_t counter, char str[], char *terminations[]);

// imprime la serie de Fibonacci con un sleep de 4 ticks
void fibonacci(uint8_t argc, void **argv)
{
    uint64_t first = FIRST;
    uint64_t second = SECOND;
    uint64_t third;
    char *terminations[] = {"st", "nd", "rd", "th"};
    char str[STRING_LENGTH] = {0};
    printString("0 1st");   // imprime los primeros aparte
    newLine();
    sys_sleep(SLEEP_TICKS);
    printString("1 2nd");
    newLine();
    sys_sleep(SLEEP_TICKS);
    uint8_t numSerie = 3;
    while (1)
    {
        third = printNewValue(first, second, numSerie, str, terminations);
        first = second;
        second = third;
        numSerie++;
        sys_sleep(SLEEP_TICKS);
    }
}

uint64_t printNewValue(uint64_t value1, uint64_t value2, uint8_t counter, char str[], char *terminations[])
{   // realiza la impresión
    uint64_t value3 = value1 + value2;
    ulongToStr(value3, str);
    printString(str);
    ulongToStr(counter, str);
    putChar(' ');
    printString(str);
    uint8_t termination = counter % 10 - 1;
    termination = (termination >= 3) ? 3 : termination;
    termination = (counter%100 >=11 && counter%100 <= 13)? 3 : termination; //el 11 12 y 13 son con th
    printString(terminations[termination]);
    newLine();
    return value3;
}

