#include "stdafx.h"
#include "itoa_64.h"
#include "Interrupts.h"
#include "Console.h"

typedef	struct {
	uint32_t Flags;

	uint32_t MemLower;
	uint32_t MemUpper;

	uint32_t BootDevice;
	uint32_t CmdLine;

	uint32_t ModsCount;
	uint32_t ModsAddr;

	// TODO: https://www.gnu.org/software/grub/manual/multiboot/multiboot.html#Boot-information-format
} MULTIBOOT_INFO;

struct {
	int32_t ConX;
	int32_t ConY;
	int32_t Multiboot;
} *Data;

void main() {
	console_init(Data->ConX, Data->ConY);
	TRACE("Long mode");

	init_interrupts();
	TRACE("Interrupts enabled");

	//__writecr0(__readcr0() & ~(0x80000001));

	TRACE("Done!");
	while (1)
		;
}