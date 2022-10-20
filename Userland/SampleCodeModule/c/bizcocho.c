#include <bizcocho.h>

void bizcochito(uint8_t argc, void** argv);

void bizcocho(uint8_t argc, void** argv)
{
    sys_clear_screen();
    char* args[1] = {"AAAAA", "BBBBB"};
    int bizcochito1pid = sys_start_child_process("bizcochito", 1, args, bizcochito);
    sys_change_priority(bizcochito1pid, 4);
    int bizcochito2pid = sys_start_child_process("bizcochito", 2, args, bizcochito);
    sys_change_priority(bizcochito2pid, 0);
    char buf[2] = {0};
    while(1)
    {
        sys_read(STDIN, buf, 1);
        sys_write(STDOUT, buf);
    }
}

void bizcochito(uint8_t argc, void** argv)
{
    long i = 0;
    while(1)
    {
        if(i++ % 0x1000000 == 0)
            sys_write(argc, "A");
    }
}