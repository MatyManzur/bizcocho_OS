
#include "testing_utils.h"
#include "syslib.h"

#define MINOR_WAIT 1000000 // TODO: Change this value to prevent a process from flooding the screen
#define WAIT      10000000 // TODO: Change this value to make the wait long enough to see theese processes beeing run at least twice

#define TOTAL_PROCESSES 3
#define LOWEST 0 //TODO: Change as required
#define MEDIUM 2 //TODO: Change as required
#define HIGHEST 4 //TODO: Change as required

int64_t prio[TOTAL_PROCESSES] = {LOWEST, MEDIUM, HIGHEST};

void test_prio(){
    int64_t pids[TOTAL_PROCESSES];
    char *argv[] = {0};
    uint64_t i;

    for(i = 0; i < TOTAL_PROCESSES; i++)
        pids[i] = sys_start_child_process("endless_loop_print", 0, argv, (int8_t (*)(uint8_t,  void **)) endless_loop_print);

    bussy_wait(WAIT);
    printf("\nCHANGING PRIORITIES...\n");

    for(i = 0; i < TOTAL_PROCESSES; i++)
        sys_change_priority(pids[i], prio[i]);

    bussy_wait(WAIT);
    printf("\nBLOCKING...\n");

    for(i = 0; i < TOTAL_PROCESSES; i++)
        sys_block_process(pids[i]);

    printf("CHANGING PRIORITIES WHILE BLOCKED...\n");

    for(i = 0; i < TOTAL_PROCESSES; i++)
        sys_change_priority(pids[i], MEDIUM);

    printf("UNBLOCKING...\n");

    for(i = 0; i < TOTAL_PROCESSES; i++)
        sys_block_process(pids[i]);

    bussy_wait(WAIT);
   printf("\nKILLING...\n");

    for(i = 0; i < TOTAL_PROCESSES; i++)
        sys_kill_process(pids[i]);

    sys_exit(0);
}