// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include <userlib.h>

#define PRINTF_BUFFER_MAX_LENGTH 255

#define IS_DIGIT(x) ((x) >= '0' && (x) <= '9')

int strToNum(const char *str)
{
    int i = 0;
    int neg = 0;
    if (str[i] == '-')
    {
        neg = 1;
        i++;
    }
    int ans = 0;
    for (; str[i] != '\0'; i++)
    {
        if (str[i] < '0' || str[i] > '9')
        {
            return -1;
        }
        ans = ans * 10 + (str[i] - '0');
    }
    if (neg)
        ans *= -1;
    return ans;
}

// https://codebrowser.dev/linux/linux/lib/string.c.html
size_t strlen(const char *s)
{
    const char *sc;
    for (sc = s; *sc != '\0'; ++sc)
        /* nothing */;
    return sc - s;
}

// se fija si el primer string está como prefijo del segundo (ignora los espacios al principio del segundo)
// deja si afterPrefix no es null lo deja apuntando al proximo caracter de str luego de encontrar el prefijo, si no lo encontro lo pone en null
uint8_t strPrefix(const char *prefix, const char *str, char **afterPrefix)
{
    int i = 0, j = 0;
    while (str[i] == ' ')
    {
        i++;
    }
    for (; prefix[j] && str[i]; i++, j++)
    {
        if (prefix[j] != str[i])
        {
            return 0;
        }
    }
    if (afterPrefix != NULL)
        *afterPrefix = (!prefix[j]) ? (char *)str + i : NULL;
    return !prefix[j]; // en el caso de que prefix no haya terminado y str sí, devuelve 0, sino devuelve 1
}

// https://codebrowser.dev/linux/linux/lib/string.c.html
int strcmp(const char *cs, const char *ct)
{
    unsigned char c1, c2;
    while (1)
    {
        c1 = *cs++;
        c2 = *ct++;
        if (c1 != c2)
            return c1 < c2 ? -1 : 1;
        if (!c1)
            break;
    }
    return 0;
}

// variante de codigo sacado de:
// https://codebrowser.dev/linux/linux/lib/string.c.html
// Devuelve cuantos caracteres copió
size_t strcpy(char *dest, const char *src)
{
    size_t tmp = 0;
    while ((*dest++ = *src++) != '\0')
        tmp++;
    return tmp;
}

// variante de código sacado de:
// https://stackoverflow.com/questions/1735236/how-to-write-my-own-printf-in-c
static size_t _snprintf(char *buffer, size_t n, char *format, va_list *args)
{
    char *traverse;
    int64_t i;
    char *s;

    int j = 0;

    // Initializing arguments
    va_list arg;
    va_copy(arg, *args);

    for (j = 0, traverse = format; *traverse != '\0' && j < n; traverse++)
    {
        while (*traverse != '%' && *traverse != 0) // frenamos en un % o en un \0
        {
            buffer[j++] = *traverse;
            traverse++;
        }

        if (*traverse == 0)
            break;

        traverse++;

        int middleNumber = 0;

        while (IS_DIGIT(*traverse)) // leemos si hay un numero entre el % y la letra indicando la cantidad minima de cifras a mostrar para que complete con ceros adelante
        {
            middleNumber *= 10;
            middleNumber += *traverse - '0';
            traverse++;
        }

        // Fetching and executing arguments
        switch (*traverse)
        {
        case 'c':
            i = va_arg(arg, int); // Fetch char argument
            buffer[j++] = i;
            break;

        case 'd':
            i = va_arg(arg, int); // Fetch Decimal/Integer argument
            if (i < 0)
            {
                i = -i;
                buffer[j++] = '-';
            }
            j += strncpy(buffer + j, convert(i, 10, middleNumber), n - j);
            break;

        case 'o':
            i = va_arg(arg, unsigned int); // Fetch Octal representation
            j += strncpy(buffer + j, convert(i, 8, middleNumber), n - j);
            break;

        case 's':
            s = va_arg(arg, char *); // Fetch string
            int maxChars = middleNumber == 0 ? n - j : ((middleNumber < n - j) ? middleNumber : n - j);
            int copied = strncpy(buffer + j, s, maxChars);
            j += copied;
            middleNumber -= copied;
            while (middleNumber > 0 && j < n)
            {
                buffer[j++] = ' ';
                middleNumber--;
            }
            break;

        case 'x':
            i = va_arg(arg, unsigned long); // Fetch Hexadecimal representation
            j += strncpy(buffer + j, convert(i, 16, middleNumber), n - j);
            break;
        }
    }
    buffer[j] = 0;
    // Closing argument list to necessary clean-up
    va_end(arg);
    return j;
}

size_t snprintf(char *buffer, size_t n, char *format, ...)
{
    va_list args;
    va_start(args, format);
    size_t aux = _snprintf(buffer, PRINTF_BUFFER_MAX_LENGTH, format, &args);
    va_end(args);
    return aux;
}

void printf(char *format, ...)
{
    va_list args;
    va_start(args, format);
    char buffer[PRINTF_BUFFER_MAX_LENGTH] = {0};
    _snprintf(buffer, PRINTF_BUFFER_MAX_LENGTH, format, &args);
    va_end(args);
    sys_write(STDOUT, buffer);
}

void fprintf(int fd, char *format, ...)
{
    va_list args;
    va_start(args, format);
    char buffer[PRINTF_BUFFER_MAX_LENGTH] = {0};
    _snprintf(buffer, PRINTF_BUFFER_MAX_LENGTH, format, &args);
    va_end(args);
    sys_write(fd, buffer);
}

// Función auxiliar para convertir un numero a string en la base indicada (maximo base 16), minDigitCount es la cantidad minima de digitos del string, si el numero tiene menos digitos entonces completa con 0's
char *convert(unsigned int num, int base, unsigned int minDigitCount)
{
    if (base > 16)
    {
        return NULL;
    }

    static char Representation[] = "0123456789ABCDEF";
    static char buffer[50];
    char *ptr;
    int digitCount = 0;

    ptr = &buffer[49];
    *ptr = '\0';

    do
    {
        *--ptr = Representation[num % base];
        num /= base;
        digitCount++;
    } while (num != 0);

    while (digitCount < minDigitCount) // agrega ceros adelante si faltan digits
    {
        *--ptr = Representation[0];
        digitCount++;
    }

    return (ptr);
}


uint8_t ulongToStr(unsigned long num, char *ans)
{
    char aux[20]; 
    
    int i = 0;
    int k = 0;
    while (num)
    {
        aux[k++] = (num % 10) + '0';
        num /= 10;
    }
    k--;
    while (k >= 0)
    {
        ans[i++] = aux[k--];
    }
    ans[i] = '\0';
    return i;
}

// Pasa de string en hexadecimal a numero entero
int xtou64(const char *str, uint64_t *ans) // devuelve el numero por parametro porque sino C lo castea mal
{
    *ans = 0;
    int count = 0;
    while (str[count] != '\0')
    {
        *ans *= 16;
        char c = str[count];
        if (IS_DIGIT(c))
        {
            *ans += c - '0';
        }
        else if (c >= 'a' && c <= 'f')
        {
            *ans += 10 + c - 'a';
        }
        else if (c >= 'A' && c <= 'F')
        {
            *ans += 10 + c - 'A';
        }
        else
        {
            return 1;
        }
        count++;
    }
    if (count > 16)
    {
        return 1;
    }
    return 0;
}

size_t strncpy(char *dest, const char *src, size_t count)
{
    size_t n = 0;
    char *tmp = dest;
    while (((*tmp = *src) != 0) && n < count)
    {
        tmp++;
        src++;
        n++;
    }
    return n;
}

int removeBackspaces(char str[])
{
    int i = 0, j = 0;
    for (; str[j] != '\0'; j++)
    {

        if (str[j] != '\b')
        {
            str[i++] = str[j];
        }
        else
        {
            i -= i != 0;
        }
    }
    str[i] = '\0';
    return i;
}

void printSemaphoreTable()
{
    uint32_t semAmount = 0;
    semInfoPointer *semInfo = sys_get_sem_info(&semAmount);
    if (semInfo == NULL && semAmount > 0)
        return;
    printf("|---------------|----|-------|---------------------------|\n");
    printf("| semaphoreName | id | value | blocked by this semaphore |\n");
    printf("|---------------|----|-------|---------------------------|\n");
    for (int i = 0; i < semAmount; i++)
    {
        printf("| %12s  | %2d |   %2d  |", semInfo[i]->name, semInfo[i]->id, semInfo[i]->value);
        int j = 0;
        while (semInfo[i]->blocked[j])
            printf("  %d  ", semInfo[i]->blocked[j++]);
        sys_mem_free(semInfo[i]->blocked);
        sys_mem_free(semInfo[i]);
        printf("\n");
    }
    printf("|---------------|----|-------|---------------------------|\n");
    sys_mem_free(semInfo);
}

void printProcessesTable()
{
    uint32_t procAmount = 0;
    processInfoPointer *processesInfo = sys_get_process_info(&procAmount);
    if (processesInfo == NULL && procAmount > 0)
        return;
    printf("|-------------|-----|------|-------|-------|------------|-----------|\n");
    printf("| processName | pid | ppid | state | prior |  stackPtr  |  basePtr  |\n");
    printf("|-------------|-----|------|-------|-------|------------|-----------|\n");

    uint32_t index = 0;

    while (index != procAmount)
    {
        processInfoPointer process = processesInfo[index];
        printf("| %11s | %3d |  %3d |   %c   |   %1d   |  0x%7x | 0x%7x |\n",
               process->name, process->pid, process->ppid, process->status, process->priority,
               (uint64_t)process->stackPointer, (uint64_t)process->processMemStart);
        sys_mem_free(processesInfo[index]);
        index++;
    }

    printf("|-------------|-----|------|-------|-------|------------|-----------|\n");
    printf("| Total Process Count: %3d | R: Ready - F: Finished - ?: Unknown    |\n", procAmount);
    printf("| Blocked because: B: Asked - P: Empty Pipe - p: Full Pipe          |\n");
    printf("| C: Waiting Child - S: Waiting Semaphore                           |\n");
    printf("|-------------------------------------------------------------------|\n");
    sys_mem_free(processesInfo);
}
void printMemInfo()
{
    memInfoPointer meminfo = sys_get_mem_info();
    if (meminfo == NULL)
        return;
    uint32_t totalBlocks = (meminfo->memSize / meminfo->blockSize);
    uint32_t occupiedBlocks = totalBlocks - meminfo->freeBlocks;
    fprintf(STDOUT, "|-------------|-------------|--------------|\n");
    fprintf(STDOUT, "| Free Blocks | Used Blocks | Total Blocks |\n");
    fprintf(STDOUT, "|-------------|-------------|--------------|\n");
    fprintf(STDOUT, "|   %7d   |   %7d   |   %7d    |\n", meminfo->freeBlocks, occupiedBlocks, totalBlocks);
    fprintf(STDOUT, "|-------------|-------------|--------------|\n");
    fprintf(STDOUT, "| Block Size= %2d Bytes                     |\n", meminfo->blockSize);
    fprintf(STDOUT, "|-------------|-------------|--------------|\n");
    fprintf(STDOUT, "|  Free Bytes |  Used Bytes |  Total Bytes |\n");
    fprintf(STDOUT, "|-------------|-------------|--------------|\n");
    fprintf(STDOUT, "|  %8d   |  %8d   |   %8d   |\n", meminfo->freeBlocks * meminfo->blockSize, occupiedBlocks * meminfo->blockSize, totalBlocks * meminfo->blockSize);
    fprintf(STDOUT, "|-------------|-------------|--------------|\n");
    fprintf(STDOUT, "| Memalloc implemtation: %17s |\n", meminfo->system);
    fprintf(STDOUT, "|------------------------------------------|\n");
    sys_mem_free(meminfo);
}
void printPipeTable()
{
    uint32_t pipeamount = 0;
    pipeInfoPointer *infopipe = sys_get_pipe_info(&pipeamount);
    if (infopipe == NULL && pipeamount > 0)
        return;

    fprintf(STDOUT, "|------------|--------------------------|\n");
    fprintf(STDOUT, "| Pipe Name  | Characters Left To Read  |\n");
    fprintf(STDOUT, "|------------|--------------------------|\n");
    for (uint32_t i = 0; i < pipeamount; i++)
    {
        fprintf(STDOUT, "| %10s |       %4d               |\n", infopipe[i]->name, infopipe[i]->charactersLeftToRead);
    }

    fprintf(STDOUT, "|------------|-------------|------------|\n");
    fprintf(STDOUT, "| Pipe Name  | Blocked PID | Blocked by |\n");
    fprintf(STDOUT, "|------------|-------------|------------|\n");
    for (uint32_t i = 0; i < pipeamount; i++)
    {
        for (uint32_t j = 0; j < infopipe[i]->amountBlockedRead; j++)
        {
            fprintf(STDOUT, "| %10s |     %3d     |    Read    |\n", infopipe[i]->name, infopipe[i]->blockedByReading[j]);
        }

        for (uint32_t j = 0; j < infopipe[i]->amountBlockedWrite; j++)
        {
            fprintf(STDOUT, "| %10s |     %3d     |    Write   |\n", infopipe[i]->name, infopipe[i]->blockedByWriting[j]);
        }
        sys_mem_free(infopipe[i]->blockedByReading);
        sys_mem_free(infopipe[i]->blockedByWriting);
        sys_mem_free(infopipe[i]);
    }
    sys_mem_free(infopipe);
    fprintf(STDOUT, "|------------|-------------|------------|\n");
}
