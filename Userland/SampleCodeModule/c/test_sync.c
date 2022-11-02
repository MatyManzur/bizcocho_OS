
#include "testing_utils.h"
#include <syslib.h>
#include "tests.h"

#define SEM_ID "sem"
#define TOTAL_PAIR_PROCESSES 3

int64_t global;  //shared memory
int8_t use_sem;
uint32_t semId;


void slowInc(int64_t *p, int64_t inc){
  int64_t aux = *p;
  //my_yield(); This makes the race condition highly probable
  aux += inc;
  for(int i = 0; i<10000; i++);
  *p = aux;
}

uint64_t my_process_inc(uint64_t argc, char *argv[]){
  uint64_t n;
  int8_t inc;
  if (argc != 3) sys_exit(1);


  if ((n = satoi(argv[0])) <= 0) sys_exit(1);
  if ((inc = satoi(argv[1])) == 0) sys_exit(1);
  if ((use_sem = satoi(argv[2])) < 0) sys_exit(1);
  

  if (use_sem && !(semId = sys_initialize_semaphore(SEM_ID, 1))) sys_exit(1);
  
  uint32_t pid = sys_get_pid();

  printf("sem: %d %s\n", pid, argv[1]);

  uint64_t i;
  for (i = 0; i < n; i++){
    if (use_sem) sys_wait_sem(semId);
    slowInc(&global, inc);
    if (use_sem) sys_post_sem(semId);
  }
  
  printf("finished: %d  %s\n", pid, argv[1]);

  sys_exit(0);
  return 0;
}

uint64_t test_sync(uint64_t argc, char *argv[]){ //{n, use_sem, 0}
  uint8_t pids[2 * TOTAL_PAIR_PROCESSES];

  if (argc != 2) sys_exit(1);

  char * argvDec[] = {argv[0], "-1", argv[1], NULL};
  char * argvInc[] = {argv[0], "1", argv[1], NULL};

  global = 0;
  uint64_t i;
  
  for(i = 0; i < TOTAL_PAIR_PROCESSES; i++){
    pids[i] = sys_start_child_process("my_process_inc", 3,(void **) argvDec, (int8_t (*)(uint8_t,  void **))my_process_inc, 0);
    pids[i + TOTAL_PAIR_PROCESSES] = sys_start_child_process("my_process_inc", 3, (void **) argvInc, (int8_t (*)(uint8_t,  void **)) my_process_inc, 0);
  }
  
  for(i = 0; i < TOTAL_PAIR_PROCESSES; i++){
    sys_wait_child(pids[i]);
    sys_wait_child(pids[i + TOTAL_PAIR_PROCESSES]);
  }

  printf("Final value: %d\n", global);

  if (use_sem) sys_close_sem(semId);

  sys_exit(0);
  return 0;
}
