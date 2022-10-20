#ifndef FILES_H_
#define FILES_H_

#include <printing.h>
#include <keyboard.h>
#include <scheduler.h>

#define STDIN 0
#define STDOUT 1
#define STDERR 2

//SYSCALLS
int write(int fd,char* s);

uint8_t read(int fd, char* buf, uint8_t n);

int printToStdoutFormat(char *s, format_t fmt);

#endif