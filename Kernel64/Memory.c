#include "stdafx.h"
#include "Memory.h"
#include "Console.h"
#include "bitmap.h"

inline uint64_t round(uint64_t Num, uint64_t Round) {
	if (Round != 0) {
		uint64_t Rem = Num % Round;
		if (Rem != 0)
			Num += Round - Rem;
	}

	return Num;
}

inline uint64_t round_down(uint64_t Num, uint64_t Round) {
	if (Round != 0)
		return round(Num - (Round - 1), Round);
}

inline void addr_to_table(uint64_t Addr, uint64_t* PT4, uint64_t* PT3, uint64_t* PT2, uint64_t* PT1, uint64_t* Offset) {
	const uint64_t SizeMask = 512 - 1;

	if (Offset != 0)
		*Offset = Addr & (4 * KiB - 1);

	Addr = Addr >> 12;
	if (PT1 != 0)
		*PT1 = Addr & SizeMask;

	Addr = Addr >> 9;
	if (PT2 != 0)
		*PT2 = Addr & SizeMask;

	Addr = Addr >> 9;
	if (PT3 != 0)
		*PT3 = Addr & SizeMask;

	Addr = Addr >> 9;
	if (PT4 != 0)
		*PT4 = Addr & SizeMask;
}

uint64_t(*alloc_mem_func)(uint64_t Len, uint64_t Round);
void(*free_mem_func)(uint64_t Addr);

uint64_t TotalMemory;
uint64_t FreeStart;
uint64_t Free;
uint64_t PT4;

uint64_t alloc_mem(uint64_t Len, uint64_t Round) {
	if (alloc_mem_func != NULL)
		return alloc_mem_func(Len, Round);

	Free = round(Free, Round);
	uint64_t Mem = Free;
	Free += Len;
	return Mem;
}

void free_mem(uint64_t Addr) {
	if (free_mem_func != NULL)
		free_mem_func(Addr);
}

uint64_t memory_create_pagetable() {
	uint64_t TableCnt = 512;

	uint64_t* Mem = (uint64_t*)alloc_mem(TableCnt * sizeof(uint64_t), 4 * KiB);
	for (uint64_t i = 0; i < TableCnt; i++)
		Mem[i] = 0;

	return (uint64_t)Mem;
}

// Index page table properly even if it has flags
uint64_t* memory_idx_pagetable(uint64_t Tbl, uint64_t Idx) {
	Tbl = Tbl & ~(4 * KiB - 1);
	return &((uint64_t*)Tbl)[Idx];
}

// TODO: Handle 2 MiB and 4 KiB pages properly
uint64_t* memory_get_pagetable(uint64_t* PT4Ptr, uint64_t Virtual, uint8_t Type) {
	uint64_t P4 = 0;
	uint64_t P3 = 0;
	uint64_t P2 = 0;
	uint64_t P1 = 0;
	addr_to_table(Virtual, &P4, &P3, &P2, &P1, (uint64_t*)NULL);

	if (*PT4Ptr == (uint64_t)NULL)
		*PT4Ptr = memory_create_pagetable();

	uint64_t* PT3 = memory_idx_pagetable((uint64_t)PT4, P4);
	if (*PT3 == (uint64_t)NULL)
		*PT3 = memory_create_pagetable() | MEM_PAGE_PRESENT | MEM_PAGE_WRITE;

	uint64_t* PT2 = memory_idx_pagetable(*PT3, P3);
	if (*PT2 == (uint64_t)NULL)
		*PT2 = memory_create_pagetable() | MEM_PAGE_PRESENT | MEM_PAGE_WRITE;

	uint64_t* PT1 = memory_idx_pagetable(*PT2, P2);

	// if type wants 2 MiB frames, or if the frame already is 2 MiB
	if (Type == MEM_FRAME_2MiB)
		return PT1;

	if (HAS_FLAG(*PT1, MEM_PAGE_SIZEEXT) && HAS_FLAG(*PT1, MEM_PAGE_PRESENT))
		return (uint64_t*)NULL;

	if (*PT1 == (uint64_t)NULL)
		*PT1 = memory_create_pagetable() | MEM_PAGE_PRESENT | MEM_PAGE_WRITE;

	return memory_idx_pagetable(*PT1, P1);
}

// Map virtual address to physical frame frame, 2 MiB or 4 KiB
// Returns true on success
BOOL memory_map_frame(uint64_t* PT4Ptr, uint64_t Virtual, uint64_t Physical, uint8_t Type, uint64_t Flags) {
	uint64_t* E = memory_get_pagetable(PT4Ptr, Virtual, Type);
	if (E == NULL)
		return FALSE;

	*E |= Physical | MEM_PAGE_PRESENT | Flags;
	if (Type == MEM_FRAME_2MiB)
		*E |= MEM_PAGE_SIZEEXT;

	return TRUE;
}

// Identity map address, 2 MiB or 4 KiB
// Returns true on success
BOOL memory_imap_frame(uint64_t* PT4Ptr, uint64_t Address, uint8_t Type, uint64_t Flags) {
	return memory_map_frame(PT4Ptr, Address, Address, Type, Flags);
}

// Map sequential virtual addresses to sequential physical addresses, Count * FrameType in length, 2 MiB or 4 KiB pages
// Returns false if one or more pages could not be mapped
BOOL memory_map_frames(uint64_t* PT4Ptr, uint64_t Virtual, uint64_t Physical, uint8_t Type, uint64_t Count, uint64_t Flags) {
	uint64_t PageSize = 4 * KiB;
	if (Type == MEM_FRAME_2MiB)
		PageSize = 2 * MiB;

	BOOL Success = TRUE;
	for (uint64_t i = 0; i < Count; i++) {
		uint64_t Offset = i * PageSize;
		if (!memory_map_frame(PT4Ptr, Virtual + Offset, Physical + Offset, Type, Flags))
			Success = FALSE;
	}

	return Success;
}

// Identity map sequential addresses, Count * FrameType in length, 2 MiB or 4 KiB pages
// Returns false if one or more pages could not be mapped
BOOL memory_imap_frames(uint64_t* PT4Ptr, uint64_t Address, uint8_t Type, uint64_t Count, uint64_t Flags) {
	return memory_map_frames(PT4Ptr, Address, Address, Type, Count, Flags);
}

void memory_paging_init(MapQueue* Queue, uint64_t Count) {
	for (uint64_t i = 0; i < Count; i++) {
		uint64_t PageSize = 4 * KiB;
		if (Queue[i].Type == MEM_FRAME_2MiB)
			PageSize = 2 * MiB;

		uint64_t Start = round_down(Queue[i].Start, PageSize);
		uint64_t Len = Queue[i].Len + (Queue[i].Start - Start);

		uint64_t Cnt = Len / PageSize;
		if (Len % PageSize != 0)
			Cnt++;

		TRACE("Mapping ");
		console_writehex(Start);
		console_write(", ");
		console_writedec(Cnt);
		console_write(" * ");
		if (Queue[i].Type == MEM_FRAME_2MiB)
			console_write("2 MiB");
		else
			console_write("4 KiB");
		console_write("\n");

		memory_imap_frames(&PT4, Start, Queue[i].Type, Cnt, MEM_PAGE_WRITE);
	}

	BREAKPOINT();
	_disable();
	__writecr3((uint64_t)PT4);
	_enable();

	TRACELN("Paging initialized");
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

void memory_init(MULTIBOOT_INFO_MMAP* MMapInfo, MapQueue* Queue, uint64_t Count) {
	TotalMemory = 0;
	FreeStart = 0;
	Free = 0;
	alloc_mem_func = NULL;
	free_mem_func = NULL;
	PT4 = (uint64_t)NULL;
	FreeStart = Free = Queue[0].Start + Queue[0].Len;

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

				/*if (FreeStart == 0 && Type == MEM_FREE && Base > 0 && Len > MiB) {
					FreeStart = Base;
					Free = Base;
				}*/
			}
			else
				memory_add(Base, Len, Type);

			Start += MMapEntry->Size + 4;
		}

		// After recalculating memory size
		if (i == 0)
			memory_paging_init(Queue, Count);
	}

	TRACELN("Memory initialized");
}