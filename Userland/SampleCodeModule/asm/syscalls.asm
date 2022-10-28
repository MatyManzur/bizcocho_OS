GLOBAL sys_exit
GLOBAL sys_start_parent_process
GLOBAL sys_start_child_process
GLOBAL sys_unblock_process
GLOBAL sys_block_process
GLOBAL sys_kill_process
GLOBAL sys_get_pid
GLOBAL sys_wait_child
GLOBAL sys_change_priority
GLOBAL sys_yield
GLOBAL sys_read
GLOBAL sys_write
GLOBAL sys_clear_screen
GLOBAL sys_change_color
GLOBAL sys_print_to_stdout_color
GLOBAL sys_set_backspace_base
GLOBAL sys_mkpipe
GLOBAL sys_open
GLOBAL sys_close
GLOBAL sys_memory_dump
GLOBAL sys_get_last_registers
GLOBAL sys_clean_buffer
GLOBAL sys_get_current_date_time
GLOBAL sys_set_time_zone
GLOBAL sys_ticks_elapsed
GLOBAL sys_seconds_elapsed
GLOBAL sys_sleep
GLOBAL sys_mem_alloc
GLOBAL sys_mem_free
GLOBAL sys_initialize_semaphore
GLOBAL sys_wait_sem
GLOBAL sys_post_sem
GLOBAL sys_close_sem

SECTION .text

%macro make_syscall 1
	push rbp
	mov rbp, rsp

	mov rax, %1
	int 88h
	
	mov rsp, rbp
	pop rbp
	ret
%endmacro

sys_exit: 
	make_syscall 0

sys_start_parent_process:
	make_syscall 1

sys_start_child_process:
	make_syscall 2
	
sys_unblock_process:
	make_syscall 3
	
sys_block_process:
	make_syscall 4

sys_kill_process:
	make_syscall 5

sys_get_pid:
	make_syscall 6

sys_wait_child:
	make_syscall 7

sys_change_priority:
	make_syscall 8

sys_yield:
	make_syscall 9
	
sys_read:
	make_syscall 10

sys_write:
	make_syscall 11

sys_clear_screen:
	make_syscall 12

sys_change_color:
	make_syscall 13

sys_print_to_stdout_color:
	make_syscall 14

sys_set_backspace_base:
	make_syscall 15

sys_mkpipe:
	make_syscall 16

sys_open:
	make_syscall 17

sys_close:
	make_syscall 18
	
sys_clean_buffer:
	make_syscall 22

sys_get_current_date_time:
	make_syscall 30
	
sys_set_time_zone:
	make_syscall 31

sys_ticks_elapsed:
	make_syscall 32
	
sys_seconds_elapsed:
	make_syscall 33
	
sys_sleep:
	make_syscall 34

sys_mem_alloc:
	make_syscall 35

sys_mem_free:
	make_syscall 36

sys_memory_dump:
	make_syscall 37
	
sys_get_last_registers:
	make_syscall 38

sys_initialize_semaphore:
	make_syscall 40

sys_wait_sem:
	make_syscall 41

sys_post_sem:
	make_syscall 42

sys_close_sem:
	make_syscall 43


	
