section .text

extern int_handler2

global testint
testint:
	;xchg bx, bx
	int 8
	ret

%macro INT_HANDLER_NOERR 1
	global int_handler_%1
	int_handler_%1:
		cli
		push dword 0
		push qword %1
		jmp int_handler
%endmacro

%macro INT_HANDLER_ERR 1
	global int_handler_%1
	int_handler_%1:
		cli
		push qword %1
		jmp int_handler

%endmacro

global int_handler
int_handler:
	push rdx
	push rcx	
	mov dword edx, [esp+16] ; Error
	mov dword ecx, [esp+12] ; IntNum

	; Push all
	push rax
	push rbx
	push rsi
	push rdi
	push rbp
	push rsp
	push r8
	push r9
	push r10
	push r11
	push r12
	push r13
	push r14
	push r15

	call int_handler2
	xchg bx, bx

	; Pop all
	pop r15
	pop r14
	pop r13
	pop r12
	pop r11
	pop r10
	pop r9
	pop r8
	pop rsp
	pop rbp
	pop rdi
	pop rsi
	pop rbx
	pop rax
	pop rcx
	pop rdx
	add esp, 8 ; Pop 2 ints
	sti
	iretq

INT_HANDLER_NOERR 0
INT_HANDLER_NOERR 1
INT_HANDLER_NOERR 2
INT_HANDLER_NOERR 3
INT_HANDLER_NOERR 4
INT_HANDLER_NOERR 5
INT_HANDLER_NOERR 6
INT_HANDLER_NOERR 7
INT_HANDLER_ERR 8
INT_HANDLER_NOERR 9
INT_HANDLER_ERR 10
INT_HANDLER_ERR 11
INT_HANDLER_ERR 12
INT_HANDLER_ERR 13
INT_HANDLER_ERR 14
INT_HANDLER_NOERR 15