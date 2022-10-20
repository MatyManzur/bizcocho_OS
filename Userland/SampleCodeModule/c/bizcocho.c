#include <bizcocho.h>

int bizcocho(int argc, void** argv)
{
    char* hola = "Hola Mundo!";
    struct format_t fmt = {-1, -1};
    while(1)
    {
        sys_print(hola, &fmt);
    }
}