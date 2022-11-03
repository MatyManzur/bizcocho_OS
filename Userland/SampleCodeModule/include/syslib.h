#ifndef SYSLIB_H
#define SYSLIB_H

#include <stdint.h>

#define NULL 0

#define STDIN 0
#define STDOUT 1
#define STDERR 2
#define EMPTY 3

#define MAX_PROC_NAME 64
#define MAX_SEM_NAME 32
#define MAX_PIPE_NAME_SIZE 32

typedef struct registers_t
{
    uint64_t rax;
    uint64_t rbx;
    uint64_t rcx;
    uint64_t rdx;
    uint64_t rsi;
    uint64_t rdi;
    uint64_t r8;
    uint64_t r9;
    uint64_t r10;
    uint64_t r11;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;
    uint64_t rip;
    uint64_t rbp;
    uint64_t rsp;
    uint64_t flags;
} registers_t;

typedef enum color_t
{
    DEFAULT = -1,
    BLACK = 0,
    BLUE,
    GREEN,
    CYAN,
    RED,
    MAGENTA,
    BROWN,
    L_GRAY,
    D_GRAY,
    L_BLUE,
    L_GREEN,
    L_CYAN,
    L_RED,
    PINK,
    YELLOW,
    WHITE
} color_t;

typedef struct format_t
{
    color_t backgroundColor;
    color_t characterColor;
}format_t;

typedef struct point_t
{
    uint8_t row;
    uint8_t column;
} point_t;

struct datetime_t
{
    uint8_t hours;
    uint8_t mins;
    uint8_t secs;

    uint8_t day;
    uint8_t month;
    uint16_t year;
};
struct timezone_t
{
    int8_t hours;
    int8_t minutes;
};

typedef struct semInfo * semInfoPointer;

typedef struct semInfo
{
    char name[MAX_SEM_NAME];
    uint32_t id;
    uint64_t value;
    uint32_t * blocked;
} semInfo;

typedef struct processInfo * processInfoPointer;

typedef struct processInfo
{
    char name[MAX_PROC_NAME];
    uint32_t pid;
    uint32_t ppid;
    char status;
    uint8_t priority;
    uint64_t stackPointer;
    void *processMemStart;
} processInfo;

typedef struct memInfo{
    uint32_t memSize;
    uint32_t blockSize;
    uint32_t freeBlocks; 
}memInfo;
typedef memInfo* memInfoPointer;

typedef struct pipeInfo
{
    char name[MAX_PIPE_NAME_SIZE];
    uint16_t charactersLeftToRead;
    uint32_t * blockedByReading;
    uint32_t amountBlockedRead;
    uint32_t* blockedByWriting;
    uint32_t amountBlockedWrite;
} pipeInfo;

typedef pipeInfo* pipeInfoPointer;

void sys_exit(int8_t statusCode);

uint32_t sys_start_parent_process(char *name, uint8_t argc, void **argv, int8_t (*processCodeStart)(uint8_t, void **), uint8_t priority, uint32_t pidToCopyFds);

uint32_t sys_start_child_process(char *name, uint8_t argc, void **argv, int8_t (*processCodeStart)(uint8_t, void **), uint8_t diesOnEsc);

uint8_t sys_block_process(uint32_t pid);

uint8_t sys_kill_process(uint32_t pid);

uint32_t sys_get_pid();

int8_t sys_wait_child(uint32_t childpid);

uint8_t sys_change_priority(uint32_t pid, uint8_t newPriority);

void sys_yield();

processInfoPointer * sys_get_process_info(uint32_t * procAmount);

uint8_t sys_read(int fd, char* buf, uint8_t n);

int sys_write(int fd,char* s);

void sys_clear_screen();

format_t sys_change_color(color_t backgroundColor, color_t characterColor);

int sys_print_to_stdout_color(char* s,format_t fmt);

void sys_set_backspace_base();

int8_t sys_mkpipe(char* name);

int8_t sys_open(char* name, uint8_t mode, uint8_t* fd);

int8_t sys_close(uint8_t fd);

int8_t sys_dup2(uint8_t fromFd, uint8_t toFd);

void sys_revert_fd_replacements();

pipeInfoPointer * sys_get_pipe_info(uint32_t* pipeAmount);

void sys_clean_buffer();

void sys_get_current_date_time(struct datetime_t *datetime, struct timezone_t *tzone);

void sys_set_time_zone(const struct timezone_t *tzone);

void sys_sleep(uint64_t sleepTicks);

void* sys_mem_alloc(uint32_t nbytes);

void sys_mem_free(void *ptr );

int sys_memory_dump(uint64_t address, uint8_t buffer[]);

memInfoPointer sys_get_mem_info();

uint32_t sys_initialize_semaphore(char * name, uint64_t initialValue);

semInfoPointer * sys_get_sem_info(uint32_t * semAmount);

int64_t sys_wait_sem(uint32_t id);

void sys_post_sem(uint32_t id);

int8_t sys_close_sem(uint32_t id);

#endif
