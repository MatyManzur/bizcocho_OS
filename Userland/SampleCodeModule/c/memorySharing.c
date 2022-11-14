#include <syslib.h>
#include <userlib.h>
#include <testing_utils.h>

uint8_t readMemorySharing()
{
    uint8_t * ppointRead = sys_open_shared_memory(0);
    while(1){
        bussy_wait(100000);
        printf("%d\n", *ppointRead);
    }
}

uint8_t tryMemorySharing()
{
    sys_start_child_process("busyRead", 0, NULL, (int8_t(*)(uint8_t, void **))readMemorySharing, 0);
    uint8_t * ppointWrite = sys_open_shared_memory(0);
    printf("Escribiendo un 5:\n");
    *ppointWrite = 5;
    printf("Esperando un poco\n");
    bussy_wait(100000000);
    printf("Escribiendo un 9\n");
    *ppointWrite = 9;
    sys_exit(0);
}