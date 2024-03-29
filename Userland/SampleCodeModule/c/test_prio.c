// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "testing_utils.h"
#include "syslib.h"

#define MINOR_WAIT 1000000
#define WAIT 0x10000000

#define TOTAL_PROCESSES 3
#define LOWEST 4
#define MEDIUM 2
#define HIGHEST 0

int64_t prio[TOTAL_PROCESSES] = {LOWEST, MEDIUM, HIGHEST};

void test_prio()
{
    int64_t pids[TOTAL_PROCESSES];
    uint64_t i;

    for (i = 0; i < TOTAL_PROCESSES; i++)
    {
        pids[i] = sys_start_child_process("endless_loop_print", 0, NULL, (int8_t(*)(uint8_t, void **))endless_loop_print, 0);
        if (pids[i] == 0)
            sys_exit(1);
    }

    bussy_wait(WAIT);
    printf("\nCHANGING PRIORITIES...\n");

    for (i = 0; i < TOTAL_PROCESSES; i++)
        sys_change_priority(pids[i], prio[i]);

    bussy_wait(WAIT);
    printf("\nBLOCKING...\n");

    for (i = 0; i < TOTAL_PROCESSES; i++)
        sys_block_process(pids[i]);

    printf("\nCHANGING PRIORITIES WHILE BLOCKED...\n");

    for (i = 0; i < TOTAL_PROCESSES; i++)
        sys_change_priority(pids[i], MEDIUM);

    printf("UNBLOCKING...\n");

    for (i = 0; i < TOTAL_PROCESSES; i++)
        sys_block_process(pids[i]);

    bussy_wait(WAIT);
    printf("\nKILLING...\n");

    for (i = 0; i < TOTAL_PROCESSES; i++)
        sys_kill_process(pids[i]);

    sys_exit(0);
}