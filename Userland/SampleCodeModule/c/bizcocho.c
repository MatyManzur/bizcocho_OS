#include <bizcocho.h>

void bizcocho(uint8_t argc, void** argv)
{
    char* hola = "Hola Mundo!";
    sys_clear_screen();
    char buf[2] = {0};
    while(1)
    {
        sys_read(STDIN, buf, 1);
        sys_write(STDOUT, buf);
    }
}