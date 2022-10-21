#include <bizcocho.h>

void bizcochito(uint8_t argc, void** argv);

void bizcocho(uint8_t argc, void** argv)
{
    sys_clear_screen();
    char* args[]={"AAAAA", "BBBBB"};
    int bizcochito1pid = sys_start_child_process("bizcochito", 1, args, bizcochito);
    sys_change_priority(bizcochito1pid, 4);
    int bizcochito2pid = sys_start_child_process("bizcochito", 2, args, bizcochito);
    sys_change_priority(bizcochito2pid, 0);
    char buf[2] = {0};
    unsigned long i=0;
    int biz1blocked=0;
    int biz1alive=1;
    while(1)
    {   
        i++;
        if(biz1alive){
            if(i % 0x10000000==0 && !biz1blocked){
                
                sys_block_process(bizcochito2pid);
                biz1blocked=1;
                sys_write(STDOUT,"Biz2 bloqueado");
            }
            if(i % 0x20000000==0 && biz1blocked){
                
                sys_unblock_process(bizcochito2pid);
                biz1blocked=0;
                sys_write(STDOUT,"Biz2 desbloqueado");
            }
            if(i%0x50000000==0){
                sys_kill_process(bizcochito2pid);
                sys_write(STDERR,"BIS2 HAS BEEN SLAIN");
                biz1alive=0;
            }
        }
        // sys_read(STDIN, buf, 1);
        // sys_write(STDOUT, buf);
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