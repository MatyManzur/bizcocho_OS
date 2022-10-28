#include <files.h>



static uint16_t fileIdToGive = 3;



static ddlADT pipeFilesList;

int cmpFileName(void* a,void* b);

int cmpFileID(void* a,void* b);

static void printToScreen(char* s,format_t* format){
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
    pipeFile_t* file=(pipeFile_t* )find(pipeFilesList,cmpFileName,(void*)name);
    if(file==NULL || (mode!='W' && mode != 'R') || openFile(file->fileId,mode,fd)==-1)
    {
        return -1;//No existe el File/El modo es invalido/Hubo un error al abrir el file osea esta lleno de FDs
    }
    file->currentOpenCount++;
    return 0;
    //buscar el que tiene name, si no lo encuentra devuelve -1
    //chequea el mode, si esta mal devuelve -1
    //llama a openFile(fileId, mode, fd) de scheduler.cf
    //return 0
}
int8_t close(uint8_t fd)
{   
    uint16_t fileId=fdToFileId(fd);
    if(fileId==-1)
    {
        return -1;
    }
    pipeFile_t* file;
    uint8_t removed=0;
    toBegin(pipeFilesList);
    while(hasNext(pipeFilesList) && !removed)
    {
        file=(pipeFile_t *)next(pipeFilesList);
        if(file->fileId==fileId){
            if(closeFile(fd)==-1)
            {
                return -1;
            }
            file->currentOpenCount--;
            if(!file->currentOpenCount)
            {
                remove(pipeFilesList);
            }
            removed=1;
        }
    }
    if(!removed)
    {
        return -1;
    }
    return 0;
}

int write(int fd,char* s)
{   
    format_t format;
    switch (fd)
    {
        case EMPTY:
            SEND_ERROR("write to EMPTY file");
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
            break;
    }
    return 0;
}

//lee hasta un \n o hasta leer n chars. devuelve cuantos leyó
uint8_t read(int fd, char* buf, uint8_t n)
{
    uint8_t totalChars = 0;
    switch (fd)
    {
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
        case EMPTY:
            SEND_ERROR("read from EMPTY file");
            break;
        case STDOUT:
            SEND_ERROR("read from STDOUT");
            break;
        case STDERR:
            SEND_ERROR("read from STDERR");
            break;
        break;
        default:
            break;
    }
    return 0;
}

int printToStdoutFormat(char *s, format_t fmt){
    printToScreen(s,&fmt);
}
int cmpFileName(void* a,void* b){
    return strcmp( ((pipeFile_t*)a)->name, ((pipeFile_t*)b)->name);
}
int cmpFileID(void* a,void* b){
    return *((uint16_t *)a)==*((uint16_t *) b);
}
int8_t increaseOpenCount(uint16_t fileID){

    pipeFile_t* file=find(pipeFilesList,cmpFileID,(void *) fileID);
    if(file==NULL){
        return -1;
    }
    file->currentOpenCount++;
    return 0;
}