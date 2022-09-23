#ifndef COMMANDS_H
#define COMMANDS_H

#include <userlib.h>
#include <syslib.h>


void printRegisters(uint8_t argc, void **argv);

void help(uint8_t argc, void **argv);

void time(uint8_t argc, void **argv);

void printmem(uint8_t argc, void **argv);

void divZero(uint8_t argc, void **argv);

void invalidOpcode(uint8_t argc, void **argv);

#endif
