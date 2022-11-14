// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <files.h>
#define PRINTF_BUFFER_MAX_LENGTH 255

#define IS_DIGIT(x) ((x) >= '0' && (x) <= '9')

#define SHM_MEM_COUNT 20
#define SHM_MEM_SIZE 128

static int16_t fileIdToGive = 3;

static ddlADT pipeFilesList;

static void* sharedMems[SHM_MEM_COUNT] = {NULL};

int cmpFileName(void *a, void *b);

int cmpFileID(void *a, void *b);

static void printToScreen(char *s, format_t *format)
{
    while (s != NULL && *s != '\0')
    {
        s = print(s, format);
        if (s != NULL)
        {
            scrollUp(1);
        }
    }
}

void initializeFiles()
{
    pipeFilesList = newList();
}

void* openSharedMem(uint8_t sharedMemId) 
{
    if(sharedMemId >= SHM_MEM_COUNT)
    {
        return NULL;
    }
    if(sharedMems[sharedMemId] == NULL)
    {
        sharedMems[sharedMemId] = memalloc(SHM_MEM_SIZE);
    }
    return sharedMems[sharedMemId];
}

int8_t mkpipe(char *name)
{
    if (find(pipeFilesList, cmpFileName, (void *)name) != NULL)
    {
        return -1; // Ya existe un pipe con ese nombre
    }
    pipeFile_t *pipeFile = memalloc(sizeof(struct pipeFile_t));
    if (pipeFile == NULL)
    {
        OUT_OF_MEM_ERROR(-1);
    }
    pipeFile->fileId = fileIdToGive++;
    strncpy(pipeFile->name, name, MAX_PIPE_NAME_SIZE);
    pipeFile->readingIndex = 0;
    pipeFile->writingIndex = 0;
    pipeFile->currentOpenCount = 0;
    pipeFile->writeOpenCount = 0;
    if (add(pipeFilesList, (void *)pipeFile) < 0)
    {
        return -1;
    }
    return 0;
}

int8_t open(char *name, uint8_t mode, uint8_t *fd)
{
    pipeFile_t aux = {0};
    strncpy(aux.name, name, MAX_PIPE_NAME_SIZE);
    pipeFile_t *file = (pipeFile_t *)find(pipeFilesList, cmpFileName, (void *)&aux);
    if (file == NULL || (mode != 'W' && mode != 'R') || openFile(file->fileId, mode, fd) == -1)
    {
        return -1;
        }
    file->currentOpenCount++;
    if (mode == 'W')
        file->writeOpenCount++;
    // openFile dejó en *fd la rta
    return 0;
}

// Para que la llame el scheduler cuando está matando un proceso
int8_t closeForKilling(int16_t fileId, uint8_t mode)
{
    pipeFile_t *file;
    toBegin(pipeFilesList);
    while (hasNext(pipeFilesList))
    {
        file = (pipeFile_t *)next(pipeFilesList);
        if (file->fileId == fileId)
        {
            file->currentOpenCount--;
            if (mode == 'W')
            {
                file->writeOpenCount--;
                if (file->writeOpenCount == 0)
                {
                    BlockedReason_t unblockRead;
                    unblockRead.id = fileId;
                    unblockRead.source = PIPE_READ;
                    unblockAllProcessesBecauseReason(unblockRead);
                }
            }
            if (!file->currentOpenCount)
            {
                memfree(file);
                remove(pipeFilesList);
            }
            return 0;
        }
    }
    return -1;
}

int8_t close(uint8_t fd)
{
    uint8_t mode = 0;
    int16_t fileId = fdToFileId(fd, &mode);
    if (fileId == -1)
    {
        return -1;
    }
    pipeFile_t *file;
    toBegin(pipeFilesList);
    while (hasNext(pipeFilesList))
    {
        file = (pipeFile_t *)next(pipeFilesList);
        if (file->fileId == fileId)
        {
            if (closeFile(fd) == -1)
            {
                return -1;
            }
            file->currentOpenCount--;
            if (mode == 'W')
            {
                file->writeOpenCount--;
                if (file->writeOpenCount == 0)
                {
                    BlockedReason_t unblockRead;
                    unblockRead.id = fileId;
                    unblockRead.source = PIPE_READ;
                    unblockAllProcessesBecauseReason(unblockRead);
                }
            }
            if (!file->currentOpenCount)
            {
                memfree(file);
                remove(pipeFilesList);
            }
            return 0;
        }
    }
    return -1;
}

int write(int fd, char *s)
{
    format_t format;
    uint8_t mode = 'W';
    int16_t fileId = fdToFileId(fd, &mode);
    pipeFile_t *pipe;
    int charIndex = 0;
    switch (fileId)
    {
    case EMPTY:
        SEND_ERROR("write to this file");
        break;

    case STDIN:
        SEND_ERROR("write to STDIN");
        break;

    case STDOUT:
        format.backgroundColor = DEFAULT;
        format.characterColor = DEFAULT;
        printToScreen(s, &format);
        break;

    case STDERR:
        format.characterColor = WHITE;
        format.backgroundColor = RED;
        printToScreen(s, &format);
        break;

    default:
        pipe = find(pipeFilesList, cmpFileID, (void *)&fileId);
        if (pipe == NULL)
        {
            SEND_ERROR("write to non-existent PIPE");
            break;
        }
        BlockedReason_t blockWrite;
        blockWrite.id = fileId;
        blockWrite.source = PIPE_WRITE;

        BlockedReason_t unblockRead;
        unblockRead.id = fileId;
        unblockRead.source = PIPE_READ;

        while (s[charIndex] != '\0')
        {
            // si el pipe está lleno
            if (((pipe->readingIndex - 1) % MAX_PIPE_BUFFER_SIZE) == ((pipe->writingIndex) % MAX_PIPE_BUFFER_SIZE))
            {
                blockProcessWithReason(getPid(), blockWrite);
            }
            pipe->buffer[pipe->writingIndex++ % MAX_PIPE_BUFFER_SIZE] = s[charIndex++];
        }
        unblockAllProcessesBecauseReason(unblockRead);
        break;
    }
    return 0;
}

// lee hasta un \n o hasta leer n chars. devuelve cuantos leyó
uint8_t read(int fd, char *buf, uint8_t n)
{
    uint8_t totalChars = 0;
    uint8_t mode = 'R';
    int16_t fileId = fdToFileId(fd, &mode);
    pipeFile_t *pipe;
    BlockedReason_t blockRead;
    BlockedReason_t unblockWrite;
    switch (fileId)
    {
    case EMPTY:

        blockRead.id = -1;
        blockRead.source = PIPE_READ;
        blockProcessWithReason(getPid(), blockRead);
        break;

    case STDIN:
        while (totalChars < n)
        {
            totalChars += readPrintables(buf + totalChars, n - totalChars);
            if (totalChars > 0 && buf[totalChars - 1] == '\n')
            {
                return totalChars;
            }
            if (totalChars < n)
            {
                blockRead.id = 0;
                blockRead.source = PIPE_READ;
                blockProcessWithReason(getPid(), blockRead);
            }
        }
        return totalChars;
        break;

    case STDOUT:
        SEND_ERROR("read from STDOUT");
        break;

    case STDERR:
        SEND_ERROR("read from STDERR");
        break;

    default:
        pipe = find(pipeFilesList, cmpFileID, (void *)&fileId);
        if (pipe == NULL)
        {
            SEND_ERROR("read from non-existent PIPE");
            break;
        }
        blockRead.id = fileId;
        blockRead.source = PIPE_READ;

        unblockWrite.id = fileId;
        unblockWrite.source = PIPE_WRITE;
        while (totalChars < n)
        {
            if ((pipe->readingIndex % MAX_PIPE_BUFFER_SIZE) == (pipe->writingIndex % MAX_PIPE_BUFFER_SIZE))
            {
                if (pipe->writeOpenCount > 0)
                {
                    blockProcessWithReason(getPid(), blockRead);
                    // se despierta cuando alguien escribio algo, o cuando ya no hay mas procesos escribiendo
                    continue; // intentamos devuelta para ver qué pasó
                }
                else
                {
                    // Ya no hay procesos que tienen el lado de escritura abierto
                    buf[totalChars++] = 0;
                    return totalChars;
                }
            }
            buf[totalChars++] = pipe->buffer[pipe->readingIndex++ % MAX_PIPE_BUFFER_SIZE];
        }
        unblockAllProcessesBecauseReason(unblockWrite);
        return totalChars;
        break;
    }
    return 0;
}

int printToStdoutFormat(char *s, format_t fmt)
{
    printToScreen(s, &fmt);
    return 0;
}

int cmpFileName(void *a, void *b)
{
    return !strcmp(((pipeFile_t *)a)->name, ((pipeFile_t *)b)->name);
}

int cmpFileID(void *a, void *b)
{
    return *((uint16_t *)a) == *((uint16_t *)b);
}

int8_t modifyOpenCount(int16_t fileID, int8_t units, uint8_t mode)
{
    pipeFile_t *file = find(pipeFilesList, cmpFileID, (void *)&fileID);
    if (file == NULL)
    {
        return -1;
    }
    file->currentOpenCount += units;
    if (mode == 'W')
        file->writeOpenCount += units;
    return 0;
}

// variante de código sacado de:
// https://stackoverflow.com/questions/1735236/how-to-write-my-own-printf-in-c
void fprintf(int fd, char *format, ...)
{
    char buffer[PRINTF_BUFFER_MAX_LENGTH] = {0};
    char *traverse;
    int64_t i;
    char *s;

    int j = 0;

    // Initializing arguments
    va_list arg;
    va_start(arg, format);

    for (j = 0, traverse = format; *traverse != '\0' && j < PRINTF_BUFFER_MAX_LENGTH; traverse++)
    {
        while (*traverse != '%' && *traverse != 0) // frenamos en un % o en un \0
        {
            buffer[j++] = *traverse;
            traverse++;
        }

        if (*traverse == 0)
            break;

        traverse++;

        int minDigitCount = 0;

        while (IS_DIGIT(*traverse)) // leemos si hay un numero entre el % y la letra indicando la cantidad minima de cifras a mostrar para que complete con ceros adelante
        {
            minDigitCount *= 10;
            minDigitCount += *traverse - '0';
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
            j += strcpy(buffer + j, convert(i, 10, minDigitCount));
            break;

        case 'o':
            i = va_arg(arg, unsigned int); // Fetch Octal representation
            j += strcpy(buffer + j, convert(i, 8, minDigitCount));
            break;

        case 's':
            s = va_arg(arg, char *); // Fetch string
            j += strcpy(buffer + j, s);
            break;

        case 'x':
            i = va_arg(arg, unsigned long); // Fetch Hexadecimal representation
            j += strcpy(buffer + j, convert(i, 16, minDigitCount));
            break;
        }
    }

    buffer[j] = 0;

    write(fd, buffer);

    // Closing argument list to necessary clean-up
    va_end(arg);
}

pipeInfoPointer *getPipeInfo(uint32_t *pipeAmount)
{

    uint32_t pipeSize = getSize(pipeFilesList);
    toBegin(pipeFilesList);
    pipeInfoPointer *informationPointer = memalloc(sizeof(pipeInfoPointer) * pipeSize);
    if (informationPointer == NULL && pipeSize > 0)
        OUT_OF_MEM_ERROR(NULL);
    uint32_t pipeIndex = 0;

    pipeFile_t *current;
    ddlADT blockedReadList = getBlockedList(PIPE_READ);
    ddlADT blockedWriteList = getBlockedList(PIPE_WRITE);

    uint32_t blockedAmount = 0;
    PCB_t *currentProcess;
    while (hasNext(pipeFilesList))
    {
        current = (pipeFile_t *)next(pipeFilesList);
        informationPointer[pipeIndex] = memalloc(sizeof(pipeInfo));
        if (informationPointer[pipeIndex] == NULL)
            OUT_OF_MEM_ERROR(NULL);
        strncpy(informationPointer[pipeIndex]->name, current->name, MAX_PIPE_NAME_SIZE);
        if (current->writingIndex % MAX_PIPE_BUFFER_SIZE < current->readingIndex % MAX_PIPE_BUFFER_SIZE)
        {
            informationPointer[pipeIndex]->charactersLeftToRead = MAX_PIPE_BUFFER_SIZE - (current->writingIndex % MAX_PIPE_BUFFER_SIZE);
        }
        else
        {
            informationPointer[pipeIndex]->charactersLeftToRead = (current->writingIndex % MAX_PIPE_BUFFER_SIZE) - (current->readingIndex % MAX_PIPE_BUFFER_SIZE);
        }
        toBegin(blockedReadList);
        blockedAmount = 0;
        while (hasNext(blockedReadList))
        {
            if (((PCB_t *)next(blockedReadList))->blockedReason.id == current->fileId)
            {
                blockedAmount++;
            }
        }
        informationPointer[pipeIndex]->blockedByReading = memalloc((blockedAmount) * sizeof(uint32_t));
        if (informationPointer[pipeIndex]->blockedByReading == NULL && blockedAmount > 0)
            OUT_OF_MEM_ERROR(NULL);
        uint32_t blockedIndex = 0;
        toBegin(blockedReadList);
        while (hasNext(blockedReadList))
        {
            currentProcess = (PCB_t *)next(blockedReadList);
            if (currentProcess->blockedReason.id == current->fileId)
            {
                informationPointer[pipeIndex]->blockedByReading[blockedIndex++] = currentProcess->pid;
            }
        }
        informationPointer[pipeIndex]->amountBlockedRead = blockedAmount;
        toBegin(blockedWriteList);
        blockedAmount = 0;
        while (hasNext(blockedWriteList))
        {
            if (((PCB_t *)next(blockedWriteList))->blockedReason.id == current->fileId)
            {
                blockedAmount++;
            }
        }
        informationPointer[pipeIndex]->blockedByWriting = memalloc((blockedAmount) * sizeof(uint32_t));
        if (informationPointer[pipeIndex]->blockedByWriting == NULL && blockedAmount > 0)
            OUT_OF_MEM_ERROR(NULL);
        blockedIndex = 0;
        toBegin(blockedWriteList);
        while (hasNext(blockedWriteList))
        {
            currentProcess = (PCB_t *)next(blockedWriteList);
            if (currentProcess->blockedReason.id == current->fileId)
            {
                informationPointer[pipeIndex]->blockedByWriting[blockedIndex++] = currentProcess->pid;
            }
        }
        informationPointer[pipeIndex]->amountBlockedWrite = blockedAmount;

        pipeIndex++;
    }
    *pipeAmount = pipeSize;
    return informationPointer;
}