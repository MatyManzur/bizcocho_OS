#include <files.h>

#define SEND_ERROR(s) {\
    write(STDERR, "Error! Cannot ");\
    write(STDERR, (s));\
    write(STDERR, "!\n");\
    return -1;};

//estas sí serían syscalls, no como las de printing.c

static void printToScreen(char* s,format_t* format){
    while(s!=NULL && *s != '\0'){
            s=print(s,format);
            if(s!=NULL){
                scrollUp(1);
            }
        }
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