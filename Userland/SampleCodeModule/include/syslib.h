#ifndef SYSLIB_H
#define SYSLIB_H

#include <stdint.h>

#define NULL 0

#define STDIN 0
#define STDOUT 1
#define STDERR 2
#define EMPTY 3

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

void sys_exit(int8_t statusCode);

uint32_t sys_start_parent_process(char *name, uint8_t argc, char **argv, int8_t(*processCodeStart)(uint8_t, void **), uint8_t priority);

uint32_t sys_start_child_process(char *name, uint8_t argc, char **argv, int8_t (*processCodeStart)(uint8_t, void **));

uint8_t sys_unblock_process(uint32_t pid);

uint8_t sys_block_process(uint32_t pid);

uint8_t sys_kill_process(uint32_t pid);

uint32_t sys_get_pid();

int8_t sys_wait_child(uint32_t childpid);

uint8_t sys_change_priority(uint32_t pid, uint8_t newPriority);

void sys_yield();

uint8_t sys_read(int fd, char* buf, uint8_t n);

int sys_write(int fd,char* s);

void sys_clear_screen();

format_t sys_change_color(color_t backgroundColor, color_t characterColor);

int sys_print_to_stdout_color(char* s,format_t fmt);

void sys_set_backspace_base();

int8_t sys_mkpipe(char* name);

int8_t sys_open(char* name, uint8_t mode, uint8_t* fd);

int8_t sys_close(uint8_t fd);

void sys_clean_buffer();

void sys_get_current_date_time(struct datetime_t *datetime, struct timezone_t *tzone);

void sys_set_time_zone(const struct timezone_t *tzone);

unsigned long sys_ticks_elapsed();

unsigned long sys_seconds_elapsed();

void sys_sleep(uint64_t sleepTicks);

void *sys_mem_alloc(uint32_t nbytes);

void sys_mem_free(void *ptr );

int sys_memory_dump(uint64_t address, uint8_t buffer[]);

void sys_get_last_registers(struct registers_t *registers);

int sys_initialize_semaphore(char * name, int initialValue);

int sys_wait_sem(int id);

void sys_post_sem(int id);

int sys_close_sem(int id);
#endif
