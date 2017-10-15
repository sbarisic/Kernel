#include "stdafx.h"

#include <stdint.h>
#include <intrin.h>

#include <string.h>

typedef struct {
	uint32_t Flags;

	uint32_t MemLower;
	uint32_t MemUpper;

	uint32_t BootDevice;
	uint32_t CmdLine;

	uint32_t ModsCount;
	uint32_t ModsAddr;

	// TODO: https://www.gnu.org/software/grub/manual/multiboot/multiboot.html#Boot-information-format
} MULTIBOOT_INFO;

MULTIBOOT_INFO* Info;
uint16_t* VidMem;

/*void write(const char* Str) {
	int Len = (int)strlen(Str);
	uint16_t* VideoMem = (uint16_t *)0xb8000;

	for (int i = 0; i < Len; i++)
		VideoMem[i] = ((uint16_t)Str[i]) | 0x0F00;
}*/

void main() {
	VidMem = (uint16_t*)0xb8000;

	for (int i = 0; i < 80 * 25; i++)
		VidMem[i] = 0;


	//write("LMAO NIGGA, IT WORKS");

	while (1)
		;
}