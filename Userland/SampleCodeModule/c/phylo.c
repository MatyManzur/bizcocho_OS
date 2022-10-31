#include <syslib.h>
#include <userlib.h>
#include <testing_utils.h>

#define MAX_PHIL 10

static uint8_t table[MAX_PHIL];
static uint8_t tableSize;
uint32_t eatingSemaphores[MAX_PHIL];

uint8_t eatingPhylo(uint8_t argc, uint8_t argv)
{
    // esto quizás explote
    uint8_t seat = argv;

    while(1){
        switch (seat%2)
        {
            // par, primero izq
        case 0:
            sys_wait_sem(eatingSemaphores[seat]);
            sys_wait_sem(eatingSemaphores[(seat+1)%tableSize]);
            break;
            // impar, primero der
        case 1:
            sys_wait_sem(eatingSemaphores[(seat+1)%tableSize]);
            sys_wait_sem(eatingSemaphores[seat]);
            break;
        default:
            break;
        }

    // quizás un semáforo de printeo para que no se vea raro, pero las chances son muy bajas
        table[seat] = 'E';
        int j = 0;
        while(j<tableSize){
            printf("%c ", table[j]);
            j++;
        }
        sys_sleep(25);
        table[seat] = '-';

        switch (seat%2)
        {
                // par, primero izq
            case 0:
                sys_post_sem(eatingSemaphores[seat]);
                sys_post_sem(eatingSemaphores[(seat+1)%tableSize]);
                break;
                // impar, primero der
            case 1:
                sys_post_sem(eatingSemaphores[(seat+1)%tableSize]);
                sys_post_sem(eatingSemaphores[seat]);
                break;
            default:
                break;
        }
    }
}

uint8_t startPhylo(uint8_t argc, char * argv[])
{
    if(argc != 1) sys_exit(1);

    uint8_t initialAmount = satoi(argv[0]);
    

    char * semNames[MAX_PHIL] = {"a", "b", "c", "d", "e", "f", "g", "h", "i", "j"};
    initialAmount %= 10;
    tableSize = initialAmount;
    int i = 0;

    while(i < initialAmount)
    {
        eatingSemaphores[i] = sys_initialize_semaphore(semNames[i], 1);
        table[i] = '-';
        i++; 
    }

    uint8_t index = 0;
    uint32_t pids[MAX_PHIL];

    while(index < initialAmount)
    {
        pids[i] = sys_start_child_process("eatingPhylo", 1, index, (int8_t (*)(uint8_t,  void **)) eatingPhylo, 1);
        index++;
    }
     uint32_t semToStop;
    uint8_t character;
    while(1)
    {
        sys_sleep(30);
        //se fija si escribieron una letra
        if(character == 'a' && tableSize < MAX_PHIL)
        {
            semToStop = (tableSize%2)?  eatingSemaphores[tableSize-1] : eatingSemaphores[0];
            sys_wait_sem(eatingSemaphores[0]);
            // si hay uno esperando en el último semáforo, habría que hacer que empiece en 0, no 1
            eatingSemaphores[tableSize] = sys_initialize_semaphore(semNames[tableSize], 1);
            pids[tableSize] = sys_start_child_process("eatingPhylo", 1, index++, (int8_t (*)(uint8_t,  void **)) eatingPhylo, 1);
            tableSize++;
            sys_post_sem(eatingSemaphores[0]);

        }
        else if(character == 'd' && tableSize>2)
        {
            semToStop = (tableSize%2)? eatingSemaphores[tableSize-1] : eatingSemaphores[0];
            sys_wait_sem(semToStop);
            // el kill es un problema si el otro lo agarró ya
            sys_kill_process(pids[tableSize-1]);
            tableSize--;
            index--;
            sys_close_sem(eatingSemaphores[tableSize]);
            sys_post_sem(semToStop);
        }
    }


    sys_exit(0);
    
}