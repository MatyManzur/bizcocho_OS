GLOBAL getSeconds
GLOBAL getMinutes
GLOBAL getHours
GLOBAL getDay
GLOBAL getMonth
GLOBAL getYear
GLOBAL getCentury


section .text


%macro getTime 1
	cli
	mov al,0x0B
	out 70h,al
	in al,71h
	or al,4
	out 71h,al


	mov al,%1
	out 70h, al
	in al, 71h
	sti
%endmacro



getSeconds:
	getTime 0x00
	ret

getMinutes:
	getTime 0x02
	ret

getHours:
	getTime 0x04
	ret

getDay:
	getTime 0x07
	ret

getMonth:
	getTime 0x08
	ret

getYear:
	getTime 0x09
	ret

getCentury:
	getTime 0x32
	ret








