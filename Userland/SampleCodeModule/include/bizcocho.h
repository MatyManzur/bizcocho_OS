#ifndef BIZCOCHO_H_
#define BIZCOCHO_H_

#include <syslib.h>
#include <userlib.h>

#define BUFFER_DIM 128
#define MAX_PIPE_TOKEN_LENGTH 30
#define MAX_TOKEN_LENGTH 15
#define MAX_TOKEN_COUNT 2
#define MAX_ARG_COUNT 8
#define MAX_PIPE_NAME_SIZE 32

typedef struct command_info{
    char* name;
    uint8_t builtin; //Es una funcion de la consola o un proceso aparte, osea permite '&'  y ' | ' o no permite

    int8_t (*programFunction)(uint8_t argc, void** argv);

}commandInfo;
int8_t bizcocho(uint8_t argc, void** argv);

#endif 