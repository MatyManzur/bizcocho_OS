// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include <stdint.h>
#include "testing_utils.h"
#include "syslib.h"

#define MAX_BLOCKS 128

typedef struct MM_rq
{
    void *address;
    uint32_t size;
} mm_rq;

uint64_t test_mm(uint64_t argc, char *argv[])
{

    mm_rq mm_rqs[MAX_BLOCKS];
    uint8_t rq;
    uint32_t total;
    uint64_t max_memory;

    if (argc != 1)
    {
        fprintf(STDERR, "testmm must receive one argument!\n");
        sys_exit(1);
    }

    if ((max_memory = satoi(argv[0])) <= 0)
    {
        fprintf(STDERR, "Error in reading max memory!\n");
        sys_exit(-1);
    }

    printf("Este Proceso es Infinito, utilizar un kill para terminarlo!\n");

    while (1)
    {
        rq = 0;
        total = 0;

        // Request as many blocks as we can
        while (rq < MAX_BLOCKS && total < max_memory)
        {
            mm_rqs[rq].size = GetUniform(max_memory - total - 1) + 1;
            mm_rqs[rq].address = sys_mem_alloc(mm_rqs[rq].size);

            // Out of blocks
            if (mm_rqs[rq].address == NULL)
            {
                break;
            }

            if (mm_rqs[rq].address)
            {
                total += mm_rqs[rq].size;
                rq++;
            }
        }

        // Set
        uint32_t i;
        for (i = 0; i < rq; i++)
            if (mm_rqs[i].address)
                memseter(mm_rqs[i].address, i, mm_rqs[i].size);

        // Check
        for (i = 0; i < rq; i++)
            if (mm_rqs[i].address)
                if (!memcheck(mm_rqs[i].address, i, mm_rqs[i].size))
                {
                    fprintf(STDERR, "test_mm ERROR\n");
                    sys_exit(1);
                }

        // Free
        for (i = 0; i < rq; i++)
            if (mm_rqs[i].address)
                sys_mem_free(mm_rqs[i].address);
    }
}
