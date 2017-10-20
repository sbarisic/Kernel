#pragma once

#define MEM_FRAME_4KiB 0
#define MEM_FRAME_2MiB 1

#define MEM_FREE 1
#define MEM_RESERVED 2
#define MEM_ACPI 3
#define MEM_HIBER 4
#define MEM_DEFECTIVE 5

#define MEM_PAGE_PRESENT		(1<<0)	
#define MEM_PAGE_WRITE			(1<<1)
#define MEM_PAGE_USER			(1<<2)
#define MEM_PAGE_WRITETROUGH	(1<<3)
#define MEM_PAGE_CACHEDISABLE	(1<<4)	
#define MEM_PAGE_ACCESSED		(1<<5)	
#define MEM_PAGE_DIRTY			(1<<6)	
#define MEM_PAGE_SIZEEXT		(1<<7)	
#define MEM_PAGE_GLOBAL			(1<<8)	
#define MEM_PAGE_AVAIL1			(1<<9)	
#define MEM_PAGE_AVAIL2			(1<<10)	
#define MEM_PAGE_AVAIL3			(1<<11) 

typedef struct {
	uint64_t Start;
	uint64_t Len;
	uint64_t Type;
} MapQeueue;

void memory_init(MULTIBOOT_INFO_MMAP* MMapInfo, MapQeueue* Queue, uint64_t Count);
void memory_add(uint64_t Base, uint64_t Len, uint32_t Type);

BOOL memory_map_frame(uint64_t* PT4Ptr, uint64_t Virtual, uint64_t Physical, uint8_t Type, uint64_t Flags);
BOOL memory_imap_frame(uint64_t* PT4Ptr, uint64_t Address, uint8_t Type, uint64_t Flags);
BOOL memory_map_frames(uint64_t* PT4Ptr, uint64_t Virtual, uint64_t Physical, uint8_t Type, uint64_t Count, uint64_t Flags);
BOOL memory_imap_frames(uint64_t* PT4Ptr, uint64_t Address, uint8_t Type, uint64_t Count, uint64_t Flags);