#include "testing_utils.h"
#include "syslib.h"

enum State {RUNNING, BLOCKED, KILLED};

typedef struct P_rq{
  int32_t pid;
  enum State state;
}p_rq;

int64_t test_processes(uint64_t argc, char *argv[]){
  uint64_t rq;
  uint64_t alive = 0;
  uint8_t action;
  uint64_t max_processes;

  if (argc != 1) sys_exit(-1);

  if ((max_processes = satoi(argv[0])) <= 0) sys_exit(-1);
  p_rq* p_rqs = sys_mem_alloc(max_processes * sizeof(p_rq));

  if(p_rqs == NULL){
    fprintf(STDERR,"test_processes: ERROR allocating space of process pointers\n");
    sys_exit(-1);
  }

  while (1){
    // Create max_processes processes
    for(rq = 0; rq < max_processes; rq++){
      p_rqs[rq].pid = sys_start_child_process("endless_loop", 0, NULL, (int8_t (*)(uint8_t,  void **)) endless_loop_print, 0);
      if (p_rqs[rq].pid == -1){
        fprintf(STDERR,"test_processes: ERROR creating process\n");
        sys_exit(-1);
      }else{
        p_rqs[rq].state = RUNNING;
        alive++;
      }
    }

    // Randomly kills, blocks or unblocks processes until every one has been killed
    while (alive > 0){
      bussy_wait(0x3000000);
      for(rq = 0; rq < max_processes; rq++){
        action = GetUniform(100) % 2; 

        switch(action){
          case 0:
            if (p_rqs[rq].state == RUNNING || p_rqs[rq].state == BLOCKED){
              printf("Lo mata a %d\n", p_rqs[rq].pid);
              if (sys_kill_process(p_rqs[rq].pid) == -1){  
                fprintf(STDERR,"test_processes: ERROR killing process\n");
                sys_exit(-1);
              }
              p_rqs[rq].state = KILLED; 
              alive--;
            }
            break;

          case 1:
            if (p_rqs[rq].state == RUNNING){
              printf("Lo bloquea a %d\n", p_rqs[rq].pid);
              if(sys_block_process(p_rqs[rq].pid) == -1){
                fprintf(STDERR,"test_processes: ERROR blocking process\n");
                sys_exit(-1);
              }
              p_rqs[rq].state = BLOCKED; 
            }
            break;
        }
      }

      bussy_wait(0x10000000);
      // Randomly unblocks processes
      for(rq = 0; rq < max_processes; rq++)
        if (p_rqs[rq].state == BLOCKED && GetUniform(100) % 2){
          printf("Lo desbloquea a %d\n", p_rqs[rq].pid);
          if(sys_block_process(p_rqs[rq].pid) == -1){
            fprintf(STDERR,"test_processes: ERROR unblocking process\n");
            sys_exit(-1);
          }
          p_rqs[rq].state = RUNNING; 
        }
    }
  }
  // sys_mem_free(p_rqs); no va a salir del while(1), pero si p_rqs era un array[] provocaba que el stack sea demasiado grande y se pase de su zona de memoria
}