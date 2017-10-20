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
	uint64_t K64Start;
	uint64_t K64Len;
} *Data;

void main() {
	MULTIBOOT_INFO* Info = (MULTIBOOT_INFO*)(int64_t)Data->Multiboot;
	MULTIBOOT_INFO_MMAP* MMapInfo = (MULTIBOOT_INFO_MMAP*)(int64_t)(Data->Multiboot + 44);

	console_init(Data->ConX, Data->ConY);
	TRACELN("Long mode");

	init_interrupts();

	MapQeueue Q[2];
	// Kernel region
	Q[0].Start = Data->K64Start;
	Q[0].Len = Data->K64Len;
	Q[0].Type = MEM_FRAME_2MiB;

	// 0 -> 2 MiB
	Q[1].Start = 0;
	Q[1].Len = 2 * MiB;
	Q[1].Type = MEM_FRAME_2MiB;

	memory_init(MMapInfo, Q, sizeof(Q) / sizeof(MapQeueue));
	
	int* I = (int*)(20 * MiB);
	*I = 123;

	TRACELN("Done!");
	while (1)
		;
}