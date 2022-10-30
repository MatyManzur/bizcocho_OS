#ifndef COMMANDS_H_
#define COMMANDS_H_

#include <syslib.h>
#include <userlib.h>
#include <testing_utils.h>

#define CHECK_ARGC(argc,amount){\
    if(argc != amount){\
        fprintf(STDERR, "Error! Invalid argument count, Received %d when %d was necessary\n",argc,amount);\
        return -1;}};

int8_t help(uint8_t argc, void** argv);
int8_t cat(uint8_t argc, void** argv);
int8_t wc(uint8_t argc, void** argv);
int8_t filter(uint8_t argc, void** argv);
int8_t color(uint8_t argc, void** argv);
int8_t monke(uint8_t argc, void** argv);
int8_t loop(uint8_t argc, void** argv);

int8_t nice(uint8_t argc, void** argv);
int8_t ps(uint8_t argc, void** argv);
int8_t mem(uint8_t argc, void** argv);


#endif