#ifndef FILES_H_
#define FILES_H_

#include <printing.h>
#include <keyboard.h>
#include <scheduler.h>
#include <ddlADT.h>
#include <stringslib.h>

#define MAX_PIPE_BUFFER_SIZE 1024
#define MAX_PIPE_NAME_SIZE 32

#define EMPTY -1
#define STDIN 0
#define STDOUT 1
#define STDERR 2

void initializeFiles();

//SYSCALLS
int write(int fd,char* s);

uint8_t read(int fd, char* buf, uint8_t n);

int printToStdoutFormat(char *s, format_t fmt);

#endif