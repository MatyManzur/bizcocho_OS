GLOBAL cpuVendor
GLOBAL getScanCode
GLOBAL swapTasks
GLOBAL initializeTask
GLOBAL saveStackPointer
GLOBAL memdumpasm
GLOBAL getRegisters

section .text

getScanCode:
	push rbp
	mov rbp, rsp
	
	mov rax,0
loop:    
	in al,0x64
	mov cl,al
        and al,0x01
        cmp al,0
        je loop
        
        in al,0x60
        
        mov rsp, rbp
        pop rbp
        ret
	
cpuVendor:
	push rbp
	mov rbp, rsp

	push rbx

	mov rax, 0
	cpuid


	mov [rdi], ebx
	mov [rdi + 4], edx
	mov [rdi + 8], ecx

	mov byte [rdi+13], 0

	mov rax, rdi

	pop rbx

	mov rsp, rbp
	pop rbp
	ret
	
saveStackPointer:
	mov [rdi], rbp
	ret
	
swapTasks:
	mov rsp, rdi  		; Por como usamos saveStackPointer en rdi apunta al rbp de TimerHandler entonces lo popeamos para no perderlo 
				;y volvemos a timer handler es como hacer un desarmado de stack a mano
	pop rbp
	ret

initializeTask:

	push rax
	
	sti
	
	; Pusheamos en el stack los registros que lee el iretq
	mov rax, ss
	push rax
	push rcx 	; en rcx tenemos cuanto tiene que valer el nuevo rsp
	pushfq
	mov rax, cs
	push rax
	push rdx	; en rdx tenemos cuanto tiene que valer el nuevo rip
	
	mov rax, [rsp + 40]
	
	push rax	;hacemos el EOI
	mov al, 20h			
	out 20h, al
	pop rax
	
	;en rdi y rsi ya tenemos los argumentos de la funcion, iretq va a tomar el rip cs rflags rsp y ss del stack
	
	iretq
	
;Desde C nos deberian pasar por argumentos primero la direccion de memoria y despues una direccion a un array para dejar los 32bytes
memdumpasm:
	push rdx
	mov rdx,0
	mov rax,0
.loop:
	cmp rdx, 32
	je .fin
	mov byte al,[rdi]
	mov byte [rsi],al
	inc rdi
	inc rsi
	inc rdx
	jmp .loop
.fin:
	mov rax,1
	pop rdx
	ret
	
%macro copyRegisters 2
	mov rbx, [rax + %1*8]
	mov [rdi + %2*8], rbx
%endmacro	

getRegisters:
	push rbp
	mov rbp, rsp

	push rax
	push rbx
	
	mov rax, [rbp]  ; rbp de saveRegisters()
	mov rax, [rax]	; rbp de keyboard_handler()
	mov rax, [rax]	; rbp de int_21()
	mov rax, [rax]	; rbp de irqDispatcher()
	add rax, 16	; salteamos el rbp de irq01Handler y la direccion de retorno a irq01Handler que pusheo C
	
	; estamos apuntando con rax al r15 del pushState
	copyRegisters 0, 13	;r15
	copyRegisters 1, 12	;r14
	copyRegisters 2, 11	;r13
	copyRegisters 3, 10	;r12
	copyRegisters 4, 9	;r11
	copyRegisters 5, 8	;r10
	copyRegisters 6, 7	;r9
	copyRegisters 7, 6	;r8
	copyRegisters 8, 5	;rdi
	copyRegisters 9, 4	;rsi
	copyRegisters 10, 15	;rbp
	copyRegisters 11, 3	;rdx
	copyRegisters 12, 2	;rcx
	copyRegisters 13, 1	;rbx
	copyRegisters 14, 0	;rax
	copyRegisters 15, 14	;rip
	copyRegisters 17, 17	;flags
	copyRegisters 18, 16	;rsp
	
	pop rbx
	pop rax
	
	mov rsp, rbp
	pop rbp
	
	ret






