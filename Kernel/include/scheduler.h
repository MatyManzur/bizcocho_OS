#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include <printing.h>
#include <lib.h>
#include <string.h>

//SYSCALLS
uint8_t startParentProcess(char *name, uint8_t argc, char **argv, void (*processCodeStart)(uint8_t, void **), uint8_t priority);

uint8_t startChildProcess(char *name, uint8_t argc, char **argv, void (*processCodeStart)(uint8_t, void **));

uint8_t getPid();

void exit(int8_t statusCode);

uint8_t killProcess(uint8_t pid);

uint8_t blockProcess(uint8_t pid);

uint8_t unblockProcess(uint8_t pid);

uint8_t changePriority(uint8_t pid, uint8_t newPriority);

//KERNEL ONLY
void scheduler();
#endif
