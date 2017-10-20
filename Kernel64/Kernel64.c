#include "stdafx.h"
#include "itoa_64.h"
#include "Interrupts.h"
#include "Console.h"
#include "Memory.h"

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
	MULTIBOOT_INFO* Info = (MULTIBOOT_INFO*)(int64_t)Data->Multiboot;
	MULTIBOOT_INFO_MMAP* MMapInfo = (MULTIBOOT_INFO_MMAP*)(int64_t)(Data->Multiboot + 44);

	console_init(Data->ConX, Data->ConY);
	TRACELN("Long mode");

	init_interrupts();
	memory_init(MMapInfo);

	/*TRACE("Free memory ");
	console_writedec((int32_t)(FreeMem / 1048576));
	console_write(" MB\n");*/

	//__writecr0(__readcr0() & ~(0x80000001));

	TRACELN("Done!");
	while (1)
		;
}