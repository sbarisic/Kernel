#include "stdafx.h"

#define MULTIBOOT_DEFINE
#include "Kernel.h"

/*__inline uint16_t vga_entry(char C, uint16_t Fg, uint16_t Bg) {
	return (uint16_t)C | (Fg | Bg << 4) << 8;
}*/

void __cdecl kmain() {
	uint16_t* Con = (uint16_t*)0xB8000;
	

	const char* SomeStr = "Hello Kernel World!\0";

	while (*SomeStr) {
		//*Con = vga_entry(*SomeStr, 15, 0);
		*Con = (uint16_t)(*SomeStr) | (15 | 0 << 4) << 8;
		SomeStr++;
		Con++;
	}

	while (1) {
	}
}