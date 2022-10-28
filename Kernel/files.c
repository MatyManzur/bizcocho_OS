#include <files.h>
static uint16_t fileIdToGive = 3;

static ddlADT pipeFilesList;

int cmpFileName(void* a,void* b);

int cmpFileID(void* a,void* b);

static void printToScreen(char* s,format_t* format)
{
    while(s!=NULL && *s != '\0'){
            s=print(s,format);
            if(s!=NULL){
                scrollUp(1);
            }
        }
}

void initializeFiles()
{
    pipeFilesList = newList();
}

int8_t mkpipe(char* name)
{   
    if( find(pipeFilesList,cmpFileName,(void *) name)!=NULL)
    {
        return -1;//Ya existe un pipe con ese nombre
    }
    pipeFile_t* pipeFile = memalloc(sizeof(struct pipeFile_t));
    if(pipeFile == NULL)
    {
        OUT_OF_MEM_ERROR(-1);
    }
    pipeFile->fileId = fileIdToGive++;
    strncpy(pipeFile->name, name, MAX_PIPE_NAME_SIZE);
    pipeFile->readingIndex = 0;
    pipeFile->writingIndex = 0;
    pipeFile->currentOpenCount = 0;
    add(pipeFilesList, (void*) pipeFile);
    return 0;
}

int8_t open(char* name, uint8_t mode, uint8_t* fd)
{
    pipeFile_t aux = {0};
    strncpy(aux.name, name, MAX_PIPE_NAME_SIZE);
    pipeFile_t* file=(pipeFile_t* )find(pipeFilesList,cmpFileName,(void*)&aux);
    if(file==NULL || (mode!='W' && mode != 'R') || openFile(file->fileId,mode,fd)==-1)
    {
        return -1;
        //No existe el File/El modo es invalido/Hubo un error al abrir el file osea esta lleno de FDs
    }
    file->currentOpenCount++;
    //openFile dejó en *fd la rta
    return 0;
}

int8_t close(uint8_t fd)
{   
    uint16_t fileId = fdToFileId(fd, 0);
    if(fileId == -1)
    {
        return -1;
    }
    pipeFile_t* file;
    toBegin(pipeFilesList);
    while(hasNext(pipeFilesList))
    {
        file = (pipeFile_t *) next(pipeFilesList);
        if(file->fileId == fileId)
        {
            if(closeFile(fd) == -1)
            {
                return -1;
            }
            file->currentOpenCount--;
            if(!file->currentOpenCount)
            {
                memfree(file);
                remove(pipeFilesList);
            }
            return 0;
        }
    }
    return -1;
}

int write(int fd,char* s)
{   
    format_t format;
    int16_t fileId = fdToFileId(fd, 'W');
    pipeFile_t* pipe; 
    int charIndex=0;
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
            printToScreen(s,&format);
            break;

        case STDERR:
            format.characterColor = WHITE;
            format.backgroundColor = RED;
            printToScreen(s,&format);
            break;

        default:
            pipe = find(pipeFilesList, cmpFileID, (void *)&fileId);
            if(pipe == NULL)
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

            while( s[charIndex] != '\0'){
                //si el pipe está lleno
                if(((pipe->readingIndex-1) % MAX_PIPE_BUFFER_SIZE ) == ((pipe->writingIndex) % MAX_PIPE_BUFFER_SIZE))
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

//lee hasta un \n o hasta leer n chars. devuelve cuantos leyó
uint8_t read(int fd, char* buf, uint8_t n)
{
    uint8_t totalChars = 0;
    int16_t fileId = fdToFileId(fd, 'R');
    pipeFile_t* pipe;
    switch (fileId)
    {
        case EMPTY:
            SEND_ERROR("read from this file");
            break;

        case STDIN:
            while(totalChars < n)
            {
                totalChars += readPrintables(buf + totalChars, n - totalChars);
                if(totalChars > 0 && buf[totalChars-1]=='\n')
                {
                    return totalChars;
                }
                if(totalChars < n)
                {
                    BlockedReason_t block;
                    block.id = 0;
                    block.source = PIPE_READ;
                    blockProcessWithReason(getPid(), block);
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
            if(pipe==NULL){
                SEND_ERROR("read from non-existent PIPE");
                break;
            }
            BlockedReason_t blockRead;
            blockRead.id = fileId;
            blockRead.source = PIPE_READ;

            BlockedReason_t unblockWrite;
            unblockWrite.id = fileId;
            unblockWrite.source = PIPE_WRITE;
            while(totalChars < n)
            {
                if( (pipe->readingIndex%MAX_PIPE_BUFFER_SIZE)==(pipe->writingIndex % MAX_PIPE_BUFFER_SIZE) )
                {
                    blockProcessWithReason(getPid(), blockRead);
                }
                buf[totalChars++] = pipe->buffer[pipe->readingIndex++ % MAX_PIPE_BUFFER_SIZE];
            }
            unblockAllProcessesBecauseReason(unblockWrite);
            break;
    }
    return 0;
}

int printToStdoutFormat(char *s, format_t fmt)
{
    printToScreen(s,&fmt);
    return 0;
}

int cmpFileName(void* a,void* b)
{
    return !strcmp( ((pipeFile_t*)a)->name, ((pipeFile_t*)b)->name);
}

int cmpFileID(void* a,void* b)
{
    return *((uint16_t *)a)==*((uint16_t *) b);
}

int8_t modifyOpenCount(uint16_t fileID, int8_t units)
{
    pipeFile_t* file = find(pipeFilesList,cmpFileID,(void *) &fileID);
    if(file==NULL){
        return -1;
    }
    file->currentOpenCount += units;
    return 0;
}