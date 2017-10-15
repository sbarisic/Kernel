#include "stdafx.h"
#include "itoa_64.h"
#include "Interrupts.h"
#include "Console.h"

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

void testint();
void main() {
	console_init();
	TRACE("Long mode");

	init_interrupts();
	TRACE("Interrupts enabled");



	TRACE("Done!");
	while (1)
		;
}