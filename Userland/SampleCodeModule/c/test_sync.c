/*
#include "testing_utils.h"
#include <syslib.h>

#define SEM_ID "sem"
#define TOTAL_PAIR_PROCESSES 6

int64_t global;  //shared memory

void slowInc(int64_t *p, int64_t inc){
  uint64_t aux = *p;
  //my_yield(); This makes the race condition highly probable
  aux += inc;
  for(int i = 0; i<40000; i++);
  *p = aux;
}

uint64_t my_process_inc(uint64_t argc, char *argv[]){
  uint64_t n;
  int8_t inc;
  int8_t use_sem;

  if (argc != 3) return -1;

  if ((n = satoi(argv[0])) <= 0) return -1;
  if ((inc = satoi(argv[1])) == 0) return -1;
  if ((use_sem = satoi(argv[2])) < 0) return -1;

  int semId;

  // honestamente no hacemos ningún chequeo en initialize_semaphore 
  if (use_sem)
    semId = sys_initialize_semaphore(SEM_ID, 1);

  uint64_t i;
  for (i = 0; i < n; i++){
    if (use_sem) sys_wait_sem(semId);
    slowInc(&global, inc);
    if (use_sem) sys_post_sem(semId);
  }

  if (use_sem) sys_close_sem(semId);
    
  return 0;
}

uint64_t test_sync(uint64_t argc, char *argv[]){ //{n, use_sem, 0}
  uint64_t pids[2 * TOTAL_PAIR_PROCESSES];

  if (argc != 2) return -1;

  char * argvDec[] = {argv[0], "-1", argv[1], NULL};
  char * argvInc[] = {argv[0], "1", argv[1], NULL};

  global = 0;

  uint64_t i;
  for(i = 0; i < TOTAL_PAIR_PROCESSES; i++){
    // how the hell es el nuestro?
    pids[i] = sys_start_child_process("my_process_inc", 3, argvDec, (void (*)(uint8_t,  void **))my_process_inc);
    pids[i + TOTAL_PAIR_PROCESSES] = sys_start_child_process("my_process_inc", 3, argvInc, (void (*)(uint8_t,  void **)) my_process_inc);
  }

  for(i = 0; i < TOTAL_PAIR_PROCESSES; i++){
    sys_wait_child(pids[i]);
    sys_wait_child(pids[i + TOTAL_PAIR_PROCESSES]);
  }

  sys_write(STDOUT, "Final value: ");
  printNum(global);
  sys_write(STDOUT, "\n");

  return 0;
}
*/