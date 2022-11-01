#include <phylo.h>

#define MAX_PHIL 10

static uint8_t forSeat;
static uint8_t table[MAX_PHIL];
static uint8_t tableSize;
static uint32_t semForSeat;
static uint8_t secondToLastWaiting;
static uint8_t huboBorrado;
static uint8_t mustSkip;

uint32_t eatingSemaphores[MAX_PHIL];

uint8_t eatingPhylo(uint8_t argc, uint8_t * argv)
{
    uint8_t seat;

    sys_wait_sem(semForSeat);

    seat = forSeat++;
    
    sys_post_sem(semForSeat);
    uint8_t left;
    uint8_t right;

    while(1){
        left = eatingSemaphores[seat];
        right = eatingSemaphores[(seat+1)%tableSize];
        if(seat==tableSize-2)
            secondToLastWaiting = 1;
        switch (seat%2)
        {
            // par, primero izq
            case 0:
                sys_wait_sem(left);
                sys_wait_sem(right);
                break;
                // impar, primero der
            case 1:
                sys_wait_sem(right);
                sys_wait_sem(left);
                break;
            default:
                break;
        }
        if((!huboBorrado && seat==tableSize-2) || (huboBorrado && seat==tableSize-1)){
            secondToLastWaiting = 0;
        }
        if(!mustSkip || seat!=tableSize-1)
        {        
            printf("Seat number: %d taken\n", seat);
            table[seat] = 'E';
            int j = 0;
            while(j<tableSize){
                printf("%c ", table[j]);
                j++;
            }
            printf("\n");
            sys_sleep(17 + 5*seat);
            table[seat] = '-';

            switch (seat%2)
            {
                    // par, primero izq
                case 0:
                    sys_post_sem(left);
                    sys_post_sem(right);
                    break;
                    // impar, primero der
                case 1:
                    sys_post_sem(right);
                    sys_post_sem(left);
                    break;
                default:
                    break;
            }
        }
        else
        {
            mustSkip = 0;
        }
    }
}

uint8_t startPhylo(uint8_t argc, char * argv[])
{
    if(argc != 1) sys_exit(1);

    uint8_t initialAmount = satoi(argv[0]);
    
    if(initialAmount<3 || initialAmount>MAX_PHIL-1){
        fprintf(STDERR, "Please choose an amount between 3 and %d\n", MAX_PHIL-1);
        sys_exit(1);
    }

    printf("a to add, r to remove, q to quit\n");
    
    semForSeat = sys_initialize_semaphore("semForSeat", 1);

    char * semNames[MAX_PHIL] = {"a", "b", "c", "d", "e", "f", "g", "h", "i", "j"};
    
    tableSize = initialAmount;
    int i = 0;
    
    while(i < initialAmount)
    {
        eatingSemaphores[i] = sys_initialize_semaphore(semNames[i], 1);
        table[i] = '-';
        i++; 
    }

    forSeat = 0;
    uint8_t index = 0;
    uint32_t pids[MAX_PHIL];
    mustSkip = 0;
    secondToLastWaiting = 0;

    while(index < initialAmount)
    {
        pids[index] = sys_start_child_process("eatingPhylo", 0, NULL, (int8_t (*)(uint8_t,  void **)) eatingPhylo, 1);
        index++;
    }

    uint32_t semToStop;
    uint8_t character;
    while(1)
    {
        character=0;
        while(character!='a' && character!='r' && character!='q')
        {
            sys_read(STDIN, &character, 1);
        }
        if(character == 'a' && tableSize < MAX_PHIL-1)
        {
            //estoy bloqueando el primer tenedor que va a agarrar el ultimo filosofo actual
            semToStop = (tableSize%2)?  eatingSemaphores[tableSize-1] : eatingSemaphores[0];
            sys_wait_sem(semToStop);
            eatingSemaphores[tableSize] = sys_initialize_semaphore(semNames[tableSize], 1);
            pids[tableSize] = sys_start_child_process("eatingPhylo", 0, NULL, (int8_t (*)(uint8_t,  void **)) eatingPhylo, 1);
            tableSize++;
            sys_post_sem(semToStop);
        }
        else if(character == 'r' && tableSize>3)
        {
            semToStop = (tableSize%2)? eatingSemaphores[tableSize-1] : eatingSemaphores[0];
            sys_wait_sem(semToStop);
            uint8_t otherSem = (tableSize%2 == 0)? eatingSemaphores[tableSize-1] : eatingSemaphores[0];
            sys_wait_sem(otherSem);
            mustSkip += secondToLastWaiting;
            sys_kill_process(pids[tableSize-1]); // no lo estaría matando... O quizás lo revive el sem_post
            index--;
            forSeat--;
            tableSize--;
           /* if(secondToLastWaiting)
            {
                sys_post_sem(eatingSemaphores[tableSize]);
            }
            sys_post_sem(semToStop);
            sys_post_sem(otherSem);*/
            sys_post_sem(semToStop);
            sys_post_sem(otherSem);
            sys_close_sem(eatingSemaphores[tableSize]);
        } else if(character == 'q')
        {
            for(int i = 0; i < tableSize ; i++)
            {
                printf("Killing: %d\n", pids[i]);
                sys_kill_process(pids[i]);
                sys_close_sem(eatingSemaphores[i]);
            }
            sys_exit(0);
        }
    }

    
}