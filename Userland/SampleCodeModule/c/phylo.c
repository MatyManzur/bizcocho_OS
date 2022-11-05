// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <phylo.h>

#define MAX_PHIL 10
#define MIN_PHIL 3

static uint8_t forSeat;
static uint8_t table[MAX_PHIL];
static uint8_t tableSize;
static uint32_t semForSeat;
static uint8_t secondToLastWaitingInLeft;
static uint8_t mustSkip;
static uint8_t lastOneWaitingInFirstSem;
static uint8_t fatherWaiting;

uint32_t eatingSemaphores[MAX_PHIL];

static format_t highlightColor= {.backgroundColor=DEFAULT, .characterColor=PINK};

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
        { // de impar a par
            // par, primero izq
            case 0:
                if(seat==tableSize-2)
                    secondToLastWaitingInLeft = 1;
                sys_wait_sem(left);
                if(seat==tableSize-2)
                    secondToLastWaitingInLeft = 0;

                if(seat==tableSize-1)
                    lastOneWaitingInFirstSem = 1;
                sys_wait_sem(right);
                if(seat==tableSize-1)
                    lastOneWaitingInFirstSem = 0;
                break;
                // impar, primero der
            case 1:
                if(seat==tableSize-2)
                    secondToLastWaitingInLeft = 1;
                if(seat==tableSize-1)
                    lastOneWaitingInFirstSem = 1;
                sys_wait_sem(right);
                if(seat==tableSize-1)
                    lastOneWaitingInFirstSem = 0;

                
                sys_wait_sem(left);
                if(seat==tableSize-2)
                    secondToLastWaitingInLeft = 0;
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
                    if(seat==0 && !fatherWaiting)
                        lastOneWaitingInFirstSem = 0;
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
        printf("Seat number: %d released, pid: %d\n", seat, pidLocal);
        int j = 0;
        while(j<tableSize){
            printf("%c ", table[j]);
            j++;
        }
        printf("\n");
    }
}

uint8_t startPhylo(uint8_t argc, char * argv[])
{
    if(argc != 1) 
    {
        fprintf(STDERR, "Phylo must receive one argument!\n");
        sys_exit(1);
    }

    uint8_t initialAmount = satoi(argv[0]);
    
    if(initialAmount<MIN_PHIL || initialAmount>MAX_PHIL-1){
        fprintf(STDERR, "Please choose an amount between %d and %d\n", MIN_PHIL, MAX_PHIL-1);
        sys_exit(1);
    }


    sys_print_to_stdout_color("CONTROLS: \n", highlightColor);
    sys_print_to_stdout_color("'a' to add, 'r' to remove, 'q' to quit\n", highlightColor);
    
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
    fatherWaiting = 0;
    secondToLastWaitingInLeft = 0;
    lastOneWaitingInFirstSem = 0;

    while(index < initialAmount)
    {
        // para que aquel que se encuentra en el último asiento sea el de pids[tableSize-1]
        pids[initialAmount-1-index] = sys_start_child_process("eatingPhylo", 0, NULL, (int8_t (*)(uint8_t,  void **)) eatingPhylo, 0);
        if(pids[initialAmount-1-index]==0)
            sys_exit(1);
        index++;
    }

    uint32_t semToStop;
    char character;
    while(1)
    {
        sys_yield(); //para que no puedan agregar más de 1 en un quantum porque se me hace mierda me parece
        character=0;
        while(character!='a' && character!='r' && character!='q')
        {
            sys_read(STDIN, &character, 1);
        }
        if(character == 'a')
        {
            if(tableSize >= MAX_PHIL-1)
            {
                fprintf(STDERR, "Reached philosopher limit. Max: %d\n", MAX_PHIL-1);
                continue;
            }
            fatherWaiting = 1;
            sys_print_to_stdout_color("Adding philosopher...\n", highlightColor);
            //estoy bloqueando el primer tenedor que va a agarrar el ultimo filosofo actual
            semToStop = (tableSize%2)?  eatingSemaphores[tableSize-1] : eatingSemaphores[0];
            if(sys_wait_sem(semToStop)<0)
                    sys_exit(1);
            eatingSemaphores[tableSize] = sys_initialize_semaphore(semNames[tableSize], 1);
            if(eatingSemaphores[tableSize]==0)
                sys_exit(1);
            pids[tableSize] = sys_start_child_process("eatingPhylo", 0, NULL, (int8_t (*)(uint8_t,  void **)) eatingPhylo, 0);
            if(pids[tableSize]==0)
                sys_exit(1);
            tableSize++;
            sys_post_sem(semToStop);
        }
        else if(character == 'r')
        {
            if(tableSize <= MIN_PHIL)
            {
                fprintf(STDERR, "There must be at least %d philosophers\n", MIN_PHIL);
                continue;
            }
            sys_print_to_stdout_color("Removing philosopher...\n", highlightColor);
            fatherWaiting = 1;
            if(sys_wait_sem(eatingSemaphores[0])<0)
                sys_exit(1);
            if(sys_wait_sem(eatingSemaphores[tableSize-2])<0)
                sys_exit(1);
            sys_kill_process(pids[tableSize-1]);
            sys_post_sem(eatingSemaphores[tableSize-1]);
            sys_post_sem(eatingSemaphores[tableSize-1]);
            sys_close_sem(eatingSemaphores[tableSize-1]);

            printf("Slaying: %d\n", pids[tableSize-1]);
            if(lastOneWaitingInFirstSem) // hay que considerar también que quizás le hicieron el free, pero no salió
            {
                sys_post_sem(eatingSemaphores[0]);
            } 
            sys_post_sem(eatingSemaphores[0]);



            mustSkip += secondToLastWaitingInLeft;
            if(mustSkip)
                sys_post_sem(eatingSemaphores[tableSize-2]);    
            

            sys_post_sem(eatingSemaphores[tableSize-2]);
            tableSize--;
            forSeat--;
            index--;

        } else if(character == 'q')
        {
            sys_print_to_stdout_color("Ending...\n", highlightColor);
            for(int i = 0; i < tableSize ; i++)
            {
                //printf("Killing: %d\n", pids[i]);
                sys_post_sem(eatingSemaphores[i]);
                sys_post_sem(eatingSemaphores[i]);
                sys_kill_process(pids[i]);
                sys_close_sem(eatingSemaphores[i]);
            }
            sys_close_sem(semForSeat);
            sys_exit(0);
        }
        fatherWaiting = 0;
    }

    
}