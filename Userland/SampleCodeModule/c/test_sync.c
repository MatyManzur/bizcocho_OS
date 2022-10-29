
#include "testing_utils.h"
#include <syslib.h>
#include "tests.h"

#define SEM_ID "sem"
#define TOTAL_PAIR_PROCESSES 1

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
  if (argc != 3) sys_exit(1);


  //printf("%d %d %d\n", satoi(argv[0]), satoi(argv[1]), satoi(argv[2]));

  if ((n = satoi(argv[0])) <= 0) sys_exit(1);
  if ((inc = satoi(argv[1])) == 0) sys_exit(1);
  if ((use_sem = satoi(argv[2])) < 0) sys_exit(1);
  inc = 1;
  n = 5;
  use_sem=1;
  printf("qwerqwer\n");
  int semId;

  // honestamente no hacemos ningún chequeo en initialize_semaphore 
  if (use_sem){
    semId = sys_initialize_semaphore(SEM_ID, 1);
  }
  uint64_t i;
  for (i = 0; i < n; i++){
    if (use_sem) sys_wait_sem(semId);
    slowInc(&global, inc);
    if (use_sem) sys_post_sem(semId);
  }
  if (use_sem) sys_close_sem(semId);
  printf("7 %d 8\n", global);

  sys_exit(0);
}

uint64_t test_sync(uint64_t argc, char *argv[]){ //{n, use_sem, 0}
  uint8_t pids[2 * TOTAL_PAIR_PROCESSES];

  if (argc != 2) sys_exit(1);

  char * argvDec[] = {argv[0], "-1", argv[1], NULL};
  char * argvInc[] = {argv[0], "1", argv[1], NULL};

  global = 0;
  uint64_t i;
  
  for(i = 0; i < TOTAL_PAIR_PROCESSES; i++){
    pids[i] = sys_start_child_process("my_process_inc", 3, argvDec, (int8_t (*)(uint8_t,  void **))my_process_inc);
    pids[i + TOTAL_PAIR_PROCESSES] = sys_start_child_process("my_process_inc", 3, argvInc, (int8_t (*)(uint8_t,  void **)) my_process_inc);
  }
  
  for(i = 0; i < TOTAL_PAIR_PROCESSES; i++){
    printf("12123\n");
    sys_wait_child(pids[i]);
    sys_wait_child(pids[i + TOTAL_PAIR_PROCESSES]);
  }
  printf("Final value: %d\n", global);

  sys_exit(0);
}
