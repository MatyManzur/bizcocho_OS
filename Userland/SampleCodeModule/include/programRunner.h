#ifndef RUNNER_H
#define RUNNER_H

#include <syslib.h>
#include <userlib.h>

typedef struct functionPointer_t
{
    void (*function)(uint8_t argc, void **argv);
} functionPointer_t;

void runner(uint8_t argc, void **argv);


#endif
