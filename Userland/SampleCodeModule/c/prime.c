#include <prime.h>

// imprime los números primos, con un sleep de 4 tick
void prime(uint8_t argc, void **argv)
{
    uint64_t counter = 3;
    printString("2");
    char str[STRING_LENGTH] = {0};
    newLine();
    int isPrime;
    int stop;
    while (1)
    {
        isPrime = 1;
        stop = sqrt(counter);
        for (int i = 3; i < stop && isPrime; i += 2)
        {
            if (counter % i == 0)
            {
                isPrime = 0;
            }
        }
        if (isPrime)
        {
            ulongToStr(counter, str);
            printString(str);
            newLine();
            sys_sleep(SLEEP_TICKS);
        }
        counter += 2;

    }
}
