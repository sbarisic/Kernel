section .text

extern gdt_code;
extern main
extern Info

global entry
entry:
	mov esp, stack.top
	mov [Info], esi

	call setup_gdt
	call main
	ret

global setup_gdt
setup_gdt:
	lgdt [gdt.pointer]
	mov dword [gdt_code], gdt.code

	mov ax, gdt.data
	mov ss, ax
	mov ds, ax
	mov es, ax
	mov ax, 0
	mov gs, ax
	mov fs, ax

	; TODO: fix
	push qword 0
	mov dword [rsp + 4], gdt.code
	mov dword [rsp], setup_gdt.reload
	retf
.reload:
	ret


section .rdata
align 8

gdt:
	dq 0
.code: equ $ - gdt
	dq (1 << 44) | (1 << 47) | (1 << 41) | (1 << 53) | (1 << 43)
.data: equ $ - gdt
	dq (1 << 44) | (1 << 47) | (1 << 41) | (1 << 53)
.pointer:
	dw $ - gdt - 1
	dd gdt
	dd 0

section .bss
align 8
stack:
.bottom:
	resb 16384
.top: