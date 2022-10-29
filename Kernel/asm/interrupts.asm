
GLOBAL _cli
GLOBAL _sti
GLOBAL picMasterMask
GLOBAL picSlaveMask
GLOBAL haltcpu
GLOBAL _hlt
GLOBAL _int20

GLOBAL _irq00Handler
GLOBAL _irq01Handler
GLOBAL _irq02Handler
GLOBAL _irq03Handler
GLOBAL _irq04Handler
GLOBAL _irq05Handler
GLOBAL _syscallHandler

GLOBAL _exception0Handler
GLOBAL _exception6Handler

EXTERN irqDispatcher
EXTERN exceptionDispatcher

EXTERN killProcess
EXTERN startParentProcess
EXTERN startChildProcess
EXTERN getPid
EXTERN waitchild
EXTERN blockProcess
EXTERN changePriority
EXTERN exit
EXTERN yield
EXTERN printAllProcesses

EXTERN read
EXTERN write
EXTERN clearScreen
EXTERN changeColor
EXTERN printToStdoutFormat
EXTERN setBackspaceBase
EXTERN mkpipe
EXTERN open
EXTERN close
EXTERN dup2
EXTERN revertFdReplacements

EXTERN cleanBuffer

EXTERN getCurrentDateTime
EXTERN setTimeZone
EXTERN ticks_elapsed
EXTERN seconds_elapsed
EXTERN sleep

EXTERN memalloc
EXTERN memfree
EXTERN memdump
EXTERN getLastRegisters
EXTERN printMemState

EXTERN initializeSemaphore
EXTERN wait_sem
EXTERN post_sem
EXTERN close_sem
EXTERN print_all_semaphores

SECTION .text

%macro pushState 0
	push rax
	push rbx
	push rcx
	push rdx
	push rbp
	push rsi
	push rdi
	push r8
	push r9
	push r10
	push r11
	push r12
	push r13
	push r14
	push r15
%endmacro

%macro popState 0
	pop r15
	pop r14
	pop r13
	pop r12
	pop r11
	pop r10
	pop r9
	pop r8
	pop rdi
	pop rsi
	pop rbp
	pop rdx
	pop rcx
	pop rbx
	pop rax
%endmacro

%macro irqHandlerMaster 1
	cli
	pushState
	
	mov rdi, %1 ; pasaje de parametro
	call irqDispatcher

	; signal pic EOI (End of Interrupt)
	mov al, 20h			
	out 20h, al

	popState
	
	sti
	
	iretq
%endmacro



%macro exceptionHandler 1
	pushState
	
	mov rax,rsp
	add rax,112	; le resto lo que me movi con los push
	add rax,8	; me muevo 8 mas para estar en la direccion de retorno que es el rip
	mov rcx,[rax]
	push rcx
	add rax,8 ; me muevo 8 mas y este es el rsp de antes de la excepcion
	push rax
	pushfq
	mov rsi,rsp
	mov rdi, %1 ; pasaje de parametro
	call exceptionDispatcher 

	popState
	iretq
%endmacro

%macro caseSyscall 2
	cmp rax, %1
	je %2
%endmacro

_hlt:
	sti
	hlt
	ret

_cli:
	cli
	ret


_sti:
	sti
	ret

picMasterMask:
	push rbp
    mov rbp, rsp
    mov ax, di
    out	21h,al
    pop rbp
    retn

picSlaveMask:
	push    rbp
    mov     rbp, rsp
    mov     ax, di  ; ax = mascara de 16 bits
    out	0A1h,al
    pop     rbp
    retn


;8254 Timer (Timer Tick)
_irq00Handler:
	irqHandlerMaster 0

;Keyboard
_irq01Handler:
	irqHandlerMaster 1

;Cascade pic never called
_irq02Handler:
	irqHandlerMaster 2

;Serial Port 2 and 4
_irq03Handler:
	irqHandlerMaster 3

;Serial Port 1 and 3
_irq04Handler:
	irqHandlerMaster 4

;USB
_irq05Handler:
	irqHandlerMaster 5


;Zero Division Exception
_exception0Handler:
	exceptionHandler 0
	
;Invalid Opcode
_exception6Handler:
	exceptionHandler 6


;syscallHandler calls the function in c with its respective arguments
;The arguments already come in its respective registers in order from C
_syscallHandler:
	push rbp
	mov rbp,rsp
	
	cli
	caseSyscall 0,	.C0
	caseSyscall 1,	.C1
	caseSyscall 2,  .C2
	caseSyscall 3,  .C3
	caseSyscall 4,	.C4
	caseSyscall 5,	.C5
	caseSyscall 6,	.C6
	caseSyscall 7, 	.C7
	caseSyscall 8, 	.C8
	caseSyscall 9,  .C9
	caseSyscall 10, .C10
	caseSyscall 11, .C11
	caseSyscall 12, .C12
	caseSyscall 13, .C13
	caseSyscall 14, .C14
	caseSyscall 15, .C15
	caseSyscall 16, .C16
	caseSyscall 17, .C17
	caseSyscall 18, .C18
	caseSyscall 19, .C19
	caseSyscall 20, .C20
	caseSyscall 22, .C22
	caseSyscall 30, .C30
	caseSyscall 31, .C31
	caseSyscall 32, .C32
	caseSyscall 33, .C33
	caseSyscall 34, .C34
	caseSyscall 35, .C35
	caseSyscall 36, .C36
	caseSyscall 37, .C37
	caseSyscall 38, .C38
	caseSyscall 39, .C39
	caseSyscall 40, .C40
	caseSyscall 41, .C41
	caseSyscall 42, .C42
	caseSyscall 43, .C43
	caseSyscall 44, .C44
	jmp .end	;default: it does nothing
.C0:
	call exit
	jmp .end	
.C1:
	call startParentProcess
	jmp .end
.C2:
	call startChildProcess
	jmp .end
.C3:
	call blockProcess
	jmp .end
.C4:
	call killProcess
	jmp .end
.C5:
	call getPid
	jmp .end
.C6:
	call waitchild
	jmp .end
.C7:
	call changePriority
	jmp .end
.C8:
	call yield
	jmp .end
.C9:
	call printAllProcesses
	jmp .end
.C10:
	call read
	jmp .end
.C11:
	call write
	jmp .end
.C12:
	call clearScreen
	jmp .end
.C13:
	call changeColor
	jmp .end
.C14:
	call printToStdoutFormat
	jmp .end
.C15:
	call setBackspaceBase
	jmp .end
.C16:
	call mkpipe
	jmp .end
.C17:
	call open
	jmp .end
.C18:
	call close
	jmp .end
.C19:
	call dup2
	jmp .end
.C20:
	call revertFdReplacements
	jmp .end
.C22:
	call cleanBuffer
	jmp .end
.C30:
	call getCurrentDateTime
	jmp .end
.C31:
	call setTimeZone
	jmp .end
.C32:
	call ticks_elapsed
	jmp .end
.C33:
	call seconds_elapsed
	jmp .end
.C34:
	call sleep
	jmp .end
.C35:
	call memalloc
	jmp .end
.C36:
	call memfree
	jmp .end
.C37:
	call memdump
	jmp .end
.C38:
	call getLastRegisters	
	jmp .end
.C39:
	call printMemState
	jmp .end
.C40:
	call initializeSemaphore
	jmp .end
.C41:
	call wait_sem
	jmp .end
.C42:
	call post_sem
	jmp .end
.C43:
	call close_sem
	jmp .end
.C44:
	call print_all_semaphores
	jmp .end
	
.end:
	push rax ;; asi no pierdo la salida de rax
	; signal pic EOI (End of Interrupt)
	mov al, 20h			
	out 20h, al
	pop rax
	mov rsp,rbp
	pop rbp
	
	sti
	
	iretq


	


haltcpu:
	cli
	hlt
	ret

_int20:
	int 20h
	ret

SECTION .bss
	aux resq 1
