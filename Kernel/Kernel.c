#include "stdafx.h"

#define MULTIBOOT_DEFINE
#include "Kernel.h"

/*__inline uint16_t vga_entry(char C, uint16_t Fg, uint16_t Bg) {
	return (uint16_t)C | (Fg | Bg << 4) << 8;
}*/

void __cdecl kmain() {
	// TODO: Paging, GDT, Long mode, loading x64 executable kernel

	long long* Mem = 0xb8000;
	*Mem = 0x2f592f412f4b2f4f;

	while (1) {
	}
}