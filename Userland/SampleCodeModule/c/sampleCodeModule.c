// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include <bizcocho.h>

int main()
{
    sys_start_parent_process("bizcocho", 0, NULL, bizcocho, 0, 0);
    sys_yield();
    while (1)
        ;
    return 0;
}
