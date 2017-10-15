MB_PAGE_ALIGN	equ 1 << 0
MB_MEMINFO		equ 1 << 1
MB_KLUDGE		equ 1 << 16

FLAGS		equ MB_PAGE_ALIGN | MB_MEMINFO | MB_KLUDGE
MAGIC		equ 0x1BADB002
CHECKSUM	equ -(MAGIC + FLAGS)

; INFO
global _multiboot_header
global _enter_long

extern _Kernel64Main
extern _write
extern _kmain
extern _MultibootInfo
extern _PT4
extern _PT3
extern _PT2
extern _PT1
;extern _GDT


section .multiboot
align 4
_multiboot_header:
	dd MAGIC
	dd FLAGS
	dd CHECKSUM

	dd _multiboot_header
	dd _multiboot_header
	dd 0
	dd 0
	dd entry


section .text_nasm
entry:
	; Setup stack
	mov esp, stack.top

	; Setup all the pointers
	mov [_MultibootInfo], ebx
	mov dword [_PT4], tables.a
	mov dword [_PT3], tables.a
	mov dword [_PT2], tables.a
	mov dword [_PT1], tables.a
	;mov dword [_GDT], tables.a

	call _kmain

	cli
.hang:
	hlt
	jmp entry.hang
	
_enter_long:
	lgdt [gdt64.pointer]

	mov esi, _MultibootInfo
	mov eax, [_Kernel64Main]
	
	push gdt64.code
	push eax

	xchg bx, bx
	retf

section .bss
align 4
stack:
	.bottom:
		resb 16384
	.top:

tables:
.a:
	resb 0x1000 * 10

section .rdata
gdt64:
	dq 0
.code: equ $ - gdt64
	dq (1 << 44) | (1 << 47) | (1 << 41) | (1 << 43) | (1 << 53)

.pointer:
	dw $ - gdt64 - 1
	dd gdt64
	dd 0