#include "stdafx.h"
#include "Memory.h"
#include "Console.h"

#define BITS_PER_WORD (sizeof(uint8_t) * 8)
#define MAP_OFFSET(m) ((m) / BITS_PER_WORD)
#define BIT_OFFSET(b) ((b) % BITS_PER_WORD)

void bitmap_set(uint8_t* Map, uint32_t Bit) {
	Map[MAP_OFFSET(Bit)] |= 1 << BIT_OFFSET(Bit);
}

void bitmap_clear(uint8_t* Map, uint32_t Bit) {
	Map[MAP_OFFSET(Bit)] &= ~(1 << BIT_OFFSET(Bit));
}

BOOL bitmap_get(uint8_t* Map, uint32_t Bit) {
	return (Map[MAP_OFFSET(Bit)] & (1 << BIT_OFFSET(Bit))) != 0;
}

uint64_t(*alloc_mem_func)(uint64_t Len, uint64_t Round);

uint64_t TotalMemory;
uint64_t FreeStart;
uint64_t Free;

uint64_t* PT4; // 512 GiB pages
uint64_t* PT3; // 1 GiB pages
uint64_t* PT2; // 2 MiB pages
uint64_t* PT1; // 4 KiB pages

void memory_init(MULTIBOOT_INFO_MMAP* MMapInfo) {
	TotalMemory = NULL;
	FreeStart = NULL;
	Free = NULL;
	alloc_mem_func = NULL;

	// Iterate twice, first time to count all the memory, second time to map it
	for (int i = 0; i < 2; i++) {
		uint64_t Start = (uint64_t)MMapInfo->Addr;
		while (Start < ((uint64_t)MMapInfo->Addr + MMapInfo->Len)) {
			MMAP_INFO* MMapEntry = (MMAP_INFO*)Start;
			uint64_t Base = MMapEntry->BaseAddr;
			uint64_t Len = MMapEntry->Len;
			uint32_t Type = MMapEntry->Type;

			if (i == 0) {
				if (Base + Len > TotalMemory)
					TotalMemory = Base + Len;

				if (FreeStart == NULL && Type == MEM_FREE && Base > NULL && Len > MiB) {
					FreeStart = Base;
					Free = Base;
				}
			}
			else
				memory_add(Base, Len, Type, i == 0);

			Start += MMapEntry->Size + 4;
		}

		// After recalculating memory size
		if (i == 0) {
			memory_paging_init();
		}
	}

	TRACELN("Memory initialized");
}

uint64_t alloc_mem(uint64_t Len, uint64_t Round) {
	if (alloc_mem_func != NULL)
		return alloc_mem_func(Len, Round);

	if (Round != 0) {
		uint64_t Rem = Free % Round;
		if (Rem != 0)
			Free += Round - Rem;
	}

	uint64_t Mem = Free;
	Free += Len;
	return Mem;
}

uint64_t* memory_create_pagetable() {
	uint64_t TableCnt = 512;

	uint64_t* Mem = (uint64_t*)alloc_mem(TableCnt * sizeof(uint64_t), 4 * KiB);
	for (uint64_t i = 0; i < TableCnt; i++)
		Mem[i] = NULL;

	return Mem;
}

void memory_paging_init() {
	PT4 = memory_create_pagetable();

	((uint64_t**)PT4)[0] = memory_create_pagetable();

	console_write("PT4 = ");
	console_writehex((uint32_t)PT4);
	console_write("\n");

	console_write("PT4[0] = ");
	console_writehex((uint32_t)((uint64_t**)PT4)[0]);
	console_write("\n");

	/*_disable();
	__writecr3(PT4);
	_enable();*/
}

void memory_add(uint64_t Base, uint64_t Len, uint32_t Type) {
	console_write("base_addr = ");
	console_writehex((int32_t)Base);
	console_write(", length = ");
	console_writehex((int32_t)Len);
	console_write(", ");
	console_writedec((int32_t)Len / 1024);
	console_write(" KB, ");

	switch (Type)
	{
	case MEM_FREE: // Available
		console_write("available");
		break;

	case MEM_RESERVED: // Reserved
		console_write("reserved");
		break;

	case MEM_ACPI: // Available, contains ACPI info
		console_write("ACPI reclaimable");
		break;

	case MEM_HIBER: // Reserved, should be preserved for hibernation
		console_write("reserved, hiber");
		break;

	case MEM_DEFECTIVE: // Defective
		console_write("defective");
		break;

	default:
		console_writehex(Type);
		break;
	}

	console_write("\n");
}

// Allocate physical frame
void memory_alloc_frame(uint64_t Base, uint64_t Count, uint8_t Type) {

}

void memory_alloc_virtual() {

}