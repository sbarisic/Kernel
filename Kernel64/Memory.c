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
	return Num;
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

#define BitmapFrameSize (2 * MiB)
uint8_t* MemoryBitmap;
uint64_t FrameCount;
MemoryList MemMap[512];
uint16_t MemMapIdx;

void memory_mark(uint64_t Base, uint64_t Len, BOOL Mark) {
	uint64_t FrameCount = Len / BitmapFrameSize;
	if (Len % BitmapFrameSize != 0)
		FrameCount++;
	uint64_t FrameStart = Base / BitmapFrameSize;

	for (uint64_t S = 0; S < FrameCount; S++) {
		if (Mark)
			bitmap_set(MemoryBitmap, FrameStart + S);
		else
			bitmap_clear(MemoryBitmap, FrameStart + S);
	}
}

BOOL memory_get_mark(uint64_t Base) {
	uint64_t FrameStart = Base / BitmapFrameSize;
	return bitmap_get(MemoryBitmap, FrameStart);
}

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

	// if type wants 2 MiB frame
	if (Type == MEM_FRAME_2MiB) {
		// if contains 4 KiB frames, return NULL
		if (!HAS_FLAG(*PT1, MEM_PAGE_SIZEEXT) && HAS_FLAG(*PT1, MEM_PAGE_PRESENT))
			return NULL;

		return PT1;
	}

	// if type wants 4 KiB frame but this is a 2 MiB frame, return null
	if (HAS_FLAG(*PT1, MEM_PAGE_SIZEEXT) && HAS_FLAG(*PT1, MEM_PAGE_PRESENT))
		return (uint64_t*)NULL;

	if (*PT1 == (uint64_t)NULL)
		*PT1 = memory_create_pagetable() | MEM_PAGE_PRESENT | MEM_PAGE_WRITE;

	return memory_idx_pagetable(*PT1, P1);
}

// Map virtual address to physical frame, 2 MiB or 4 KiB
// Returns true on success
BOOL memory_map_frame(uint64_t* PT4Ptr, uint64_t Virtual, uint64_t Physical, uint8_t Type, uint64_t Flags) {
	uint64_t* E = memory_get_pagetable(PT4Ptr, Virtual, Type);
	if (E == NULL)
		return FALSE;

	*E |= Physical | MEM_PAGE_PRESENT | Flags;
	if (Type == MEM_FRAME_2MiB)
		*E |= MEM_PAGE_SIZEEXT;

	uint64_t FrameSize = 2 * MiB;
	if (Type == MEM_FRAME_4KiB)
		FrameSize = 4 * KiB;
	memory_mark(Virtual, FrameSize, FALSE);
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

// Allocates and identity maps a 2 MiB frame of memory
uint64_t memory_alloc_frame() {
	for (uint64_t F = 0; F < FrameCount; F++) {
		uint64_t Base = F * BitmapFrameSize;

		// Check if it's not already allocated
		if (memory_get_mark(Base)) {
			// Try to identity map it
			if (memory_imap_frame(&PT4, Base, MEM_FRAME_2MiB, MEM_PAGE_WRITE)) {
				// Success, mark it as allocated and return
				memory_mark(Base, BitmapFrameSize, FALSE);
				return Base;
			}
		}
	}

	return 0;
}

// Frees a 2 MiB frame of memory
void memory_free_frame(uint64_t Base) {
	if (!memory_get_mark(Base)) {
		memory_mark(Base, BitmapFrameSize, TRUE);
		// TODO: Unmap
	}
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
	MemoryList* Prev = NULL;
	if (MemMapIdx > 0)
		Prev = &MemMap[MemMapIdx - 1];

	MemoryList* Entry = &MemMap[MemMapIdx++];
	if (Prev != NULL)
		Prev->Next = Entry;

	Entry->Type = Type;
	Entry->Start = Base;
	Entry->Len = Len;

	if (Type == MEM_FREE)
		memory_mark(Base, Len, TRUE);
	else
		memory_mark(Base, Len, FALSE);
}

void memory_init(MULTIBOOT_INFO_MMAP* MMapInfo, MapQueue* Queue, uint64_t Count) {
	TotalMemory = 0;
	Free = 0;
	alloc_mem_func = NULL;
	free_mem_func = NULL;
	PT4 = (uint64_t)NULL;
	FreeStart = Free = Queue[0].Start + Queue[0].Len;
	memset(MemMap, 0, sizeof(MemMap));

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
			}
			else
				memory_add(Base, Len, Type);

			Start += MMapEntry->Size + 4;
		}

		// After recalculating memory size, allocate apropriate bitmap
		if (i == 0) {
			FrameCount = TotalMemory / BitmapFrameSize;
			if (TotalMemory % BitmapFrameSize != 0)
				FrameCount++;

			uint64_t BitmapByteCount = FrameCount / 8;
			if (FrameCount % 8 != 0)
				BitmapByteCount++;

			TRACE("Allocating ");
			console_writedec(BitmapByteCount);
			console_write(" B for MemoryBitmap\n");
			MemoryBitmap = (uint8_t*)alloc_mem(BitmapByteCount, 0);
		}
	}

	memory_paging_init(Queue, Count);
	TRACE("Memory initialized, ");

	uint64_t Mem = 0;
	for (uint64_t i = 0; i < FrameCount; i++)
		if (memory_get_mark(i * BitmapFrameSize))
			Mem += BitmapFrameSize;

	Mem = Mem / MiB;
	console_writedec((int32_t)Mem);
	console_write(" MiB free\n");
}