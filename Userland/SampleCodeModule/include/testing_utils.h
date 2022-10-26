
#include "syslib.h"

uint32_t GetUint();

uint32_t GetUniform(uint32_t max);

static void memset(void * beginning, uint32_t value, uint32_t size);

static int memcheck(void * beginning, uint32_t value, uint32_t size);

int64_t satoi(char* str);

void printNum(int value);
