#ifndef FILES_H_
#define FILES_H_

#include <printing.h>
#include <keyboard.h>
#include <scheduler.h>

//SYSCALLS
int write(int fd,char* s);

uint8_t read(int fd, char* buf, uint8_t n);

#endif