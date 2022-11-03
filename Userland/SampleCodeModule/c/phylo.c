#include <phylo.h>

#define MAX_PHIL 10

static uint8_t forSeat;
static uint8_t table[MAX_PHIL];
static uint8_t tableSize;
static uint32_t semForSeat;
static uint8_t secondToLastWaitingInLeft;
static uint8_t mustSkip;
static uint8_t lastOneWaitingInFirstSem;

uint32_t eatingSemaphores[MAX_PHIL];

uint8_t eatingPhylo(uint8_t argc, uint8_t * argv)
{
    uint8_t seat;
    uint32_t pidLocal = sys_get_pid();

    if(sys_wait_sem(semForSeat)<0)
        sys_exit(1);

    seat = forSeat++;
    
    sys_post_sem(semForSeat);
    uint8_t left;
    uint8_t right;

    while(1){
        left = eatingSemaphores[seat];
        right = eatingSemaphores[(seat+1)%tableSize];
        switch (seat%2)
        {
            // par, primero izq
            case 0:
                if(seat==tableSize-2)
                    secondToLastWaitingInLeft = 1;

                if(sys_wait_sem(left)<0)
                    sys_exit(1);
                if(seat==tableSize-2)
                    secondToLastWaitingInLeft = 1;

                if(seat==tableSize-1)
                    lastOneWaitingInFirstSem = 1;
                if(sys_wait_sem(right)<0)
                    sys_exit(1);
                if(seat==tableSize-1)
                    lastOneWaitingInFirstSem = 0;
                break;
                // impar, primero der
            case 1:
                if(seat==tableSize-1)
                    lastOneWaitingInFirstSem = 1;
                if(sys_wait_sem(right)<0)
                    sys_exit(1);
                if(seat==tableSize-1)
                    lastOneWaitingInFirstSem = 0;

                if(seat==tableSize-2)
                    secondToLastWaitingInLeft = 1;
                if(sys_wait_sem(left)<0)
                    sys_exit(1);
                if(seat==tableSize-2)
                    secondToLastWaitingInLeft = 1;
                break;
            default:
                break;
        }
        if(!mustSkip || seat!=tableSize-1)
        {        
            printf("Seat number: %d taken, pid: %d\n", seat, pidLocal);
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
    if(semForSeat==0)
        sys_exit(1);

    char * semNames[MAX_PHIL] = {"a", "b", "c", "d", "e", "f", "g", "h", "i", "j"};
    
    tableSize = initialAmount;
    int i = 0;
    
    while(i < initialAmount)
    {
        eatingSemaphores[i] = sys_initialize_semaphore(semNames[i], 1);
        if(eatingSemaphores[i]==0)
            sys_exit(1);
        table[i] = '-';
        i++; 
    }

    forSeat = 0;
    uint8_t index = 0;
    uint32_t pids[MAX_PHIL];
    mustSkip = 0;
    secondToLastWaitingInLeft = 0;
    lastOneWaitingInFirstSem = 0;

    while(index < initialAmount)
    {
        // para que aquel que se encuentra en el último asiento sea el de pids[tableSize-1]
        pids[initialAmount-1-index] = sys_start_child_process("eatingPhylo", 0, NULL, (int8_t (*)(uint8_t,  void **)) eatingPhylo, 1);
        if(pids[initialAmount-1-index]==0)
            sys_exit(1);
        index++;
    }

    uint32_t semToStop;
    char character;
    while(1)
    {
        //sys_yield(); para que no puedan agregar más de 1 en un quantum porque se me hace mierda me parece
        character=0;
        while(character!='a' && character!='r' && character!='q')
        {
            sys_read(STDIN, &character, 1);
        }
        if(character == 'a' && tableSize < MAX_PHIL-1)
        {
            //estoy bloqueando el primer tenedor que va a agarrar el ultimo filosofo actual
            semToStop = (tableSize%2)?  eatingSemaphores[tableSize-1] : eatingSemaphores[0];
            if(sys_wait_sem(semToStop)<0)
                    sys_exit(1);
            eatingSemaphores[tableSize] = sys_initialize_semaphore(semNames[tableSize], 1);
            if(eatingSemaphores[tableSize]==0)
                sys_exit(1);
            pids[tableSize] = sys_start_child_process("eatingPhylo", 0, NULL, (int8_t (*)(uint8_t,  void **)) eatingPhylo, 1);
            if(pids[tableSize]==0)
                sys_exit(1);
            tableSize++;
            sys_post_sem(semToStop);
        }
        else if(character == 'r' && tableSize>3)
        {
            if(sys_wait_sem(eatingSemaphores[0])<0)
                sys_exit(1);
            if(sys_wait_sem(eatingSemaphores[tableSize-2])<0)
                sys_exit(1);
            sys_post_sem(eatingSemaphores[tableSize-1]);
            sys_post_sem(eatingSemaphores[tableSize-1]);
            sys_close_sem(eatingSemaphores[tableSize-1]);
            sys_kill_process(pids[tableSize-1]); // quizás haya que matarlo antes de hacer los sem_post
            printf("Slaying: %d\n", pids[tableSize-1]);
            if(lastOneWaitingInFirstSem) // hay que considerar también que quizás le hicieron el free, pero no salió
            {
                sys_post_sem(eatingSemaphores[0]);
            } 
            sys_post_sem(eatingSemaphores[0]);

            if(tableSize%2 == 0)
            {
                mustSkip += secondToLastWaitingInLeft;
                if(mustSkip)
                    sys_post_sem(eatingSemaphores[tableSize-2]);    
            }

            sys_post_sem(eatingSemaphores[tableSize-2]);
            tableSize--;
            forSeat--;
            index--;

        } else if(character == 'q')
        {
            for(int i = 0; i < tableSize ; i++)
            {
                printf("Killing: %d\n", pids[i]);
                sys_post_sem(eatingSemaphores[i]);
                sys_post_sem(eatingSemaphores[i]);
                sys_kill_process(pids[i]);
                sys_close_sem(eatingSemaphores[i]);
            }
            sys_close_sem(semForSeat);
            sys_exit(0);
        }
    }

    
}