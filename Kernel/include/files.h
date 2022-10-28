#ifndef FILES_H_
#define FILES_H_

#include <lib.h>
#include <keyboard.h>
#include <printing.h>
#include <scheduler.h>

#define MAX_PIPE_BUFFER_SIZE 1024
#define MAX_PIPE_NAME_SIZE 32

#define EMPTY -1
#define STDIN 0
#define STDOUT 1
#define STDERR 2
#define SEND_ERROR(s) {\
    write(STDERR, "Error! Cannot ");\
    write(STDERR, (s));\
    write(STDERR, "!\n");\
    return -1;};

typedef struct pipeFile_t {
    uint16_t fileId;
    char name[MAX_PIPE_NAME_SIZE];
    char buffer[MAX_PIPE_BUFFER_SIZE];
    size_t readingIndex;
    size_t writingIndex;
    size_t currentOpenCount;
} pipeFile_t;

//KERNEL

void initializeFiles();

int8_t modifyOpenCount(uint16_t fileID, int8_t units);

//SYSCALLS
int write(int fd,char* s);

uint8_t read(int fd, char* buf, uint8_t n);

int printToStdoutFormat(char *s, format_t fmt);

int8_t mkpipe(char* name);

int8_t open(char* name, uint8_t mode, uint8_t* fd);

int8_t close(uint8_t fd);

#endif