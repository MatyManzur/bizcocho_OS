#include "testing_utils.h"
#include "syslib.h"

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

  // honestamente no hacemos ningún chequeo en initialize_semaphore 
  if (use_sem)
    if (!sys_initialize_semaphore(SEM_ID, 1)){
      printf("test_sync: ERROR opening semaphore\n");
      return -1;
    }

  uint64_t i;
  for (i = 0; i < n; i++){
    if (use_sem) sys_wait_sem(SEM_ID);
    slowInc(&global, inc);
    if (use_sem) sys_post_sem(SEM_ID);
  }

  if (use_sem) sys_close_sem(SEM_ID);
  
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
    pids[i] = my_create_process("my_process_inc", 3, argvDec);
    pids[i + TOTAL_PAIR_PROCESSES] = my_create_process("my_process_inc", 3, argvInc);
  }

  for(i = 0; i < TOTAL_PAIR_PROCESSES; i++){
    sys_wait_child(pids[i]);
    sys_wait_child(pids[i + TOTAL_PAIR_PROCESSES]);
  }

  sys_write(STDIN, "Final value: ");
  printNum(global);
  sys_write(STDIN, "\n");

  return 0;
}