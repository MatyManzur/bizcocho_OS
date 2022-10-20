#include <bizcocho.h>

void bizcocho(uint8_t argc, void** argv)
{
    char* hola = "Hola Mundo!";
    struct format_t fmt = {-1, -1};
    while(1)
    {
        sys_write(STDOUT,hola);
    }
}