#pragma once

#pragma section(".bss")
#define PAGE_TABLE __declspec(allocate(".bss")) volatile

#define MULTIBOOT_HEADER_MAGIC 0x1BADB002
#define CHECKSUM -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)

#define FLAG_AOUT_KLUDGE (1 << 16)
#define FLAG_3 (1 << 3)
#define FLAG_VIDEO (1 << 2)
#define FLAG_MEMORY_INFO (1 << 1)
#define FLAG_PAGE_ALIGN (1 << 0)

#define MULTIBOOT_HEADER_FLAGS (FLAG_AOUT_KLUDGE | FLAG_MEMORY_INFO | FLAG_PAGE_ALIGN)

typedef struct {
	uint32_t Magic;
	uint32_t Flags;
	uint32_t Checksum;

	uint32_t HeaderAddress;
	uint32_t LoadAddress;
	uint32_t LoadEndAddress;
	uint32_t BssEndAddress;
	uint32_t EntryAddress;

	uint32_t ModeType;
	uint32_t Width;
	uint32_t Height;
	uint32_t Depth;
} MULTIBOOT_HEADER;

typedef struct {
	uint32_t ModStart;
	uint32_t ModEnd;
	char* String;
	uint32_t Reserved;
} MULTIBOOT_MOD;

typedef struct {
	uint32_t Flags;
	
	uint32_t MemLower;
	uint32_t MemUpper;

	uint32_t BootDevice;
	uint32_t CmdLine;

	uint32_t ModsCount;
	MULTIBOOT_MOD* ModsAddr;

	// TODO: https://www.gnu.org/software/grub/manual/multiboot/multiboot.html#Boot-information-format
} MULTIBOOT_INFO;

void kmain();

#ifdef MULTIBOOT_DEFINE
#pragma section(".text")
__declspec(allocate(".text")) MULTIBOOT_HEADER Hdr = {
	MULTIBOOT_HEADER_MAGIC,
	MULTIBOOT_HEADER_FLAGS,
	CHECKSUM,

	(uint32_t)&Hdr,
	(uint32_t)&Hdr,
	0,
	0,
	(uint32_t)&kmain,

	0,
	800,
	600,
	32,
};
#endif


#define X86_PAGE_PRESENT	(1<<0)	// set if page or page-table is present
#define X86_PAGE_WRITE		(1<<1)	// set to allow writing to the page or page-table
#define X86_PAGE_USER		(1<<2)	// set to make the page or page-table a user page or page-table
// if this flag is cleared the page or page-table can only be accessed in supervisor mode
#define X86_PAGE_PWT		(1<<3)	// when the PWT flag is set, write-through caching is enabled for the
// page or page-table; when the flag is clear, write-back caching is
// enabled for the page or page-table.
// This type of caching is useful for VGA framebuffers.
#define X86_PAGE_PCD		(1<<4)	// Page Cache Disable, set this flag to disable caching of the page or page-table
#define X86_PAGE_ACCESSED	(1<<5)	// the processor sets this flag the first time the page or page table is accessed
#define X86_PAGE_DIRTY		(1<<6)	// the processor sets this flag the first time a page is accessed for a write operation
// this flag is only used in page-table entries and should not be set for page-directory entries
#define X86_PAGE_PAGESIZE	(1<<7)	// page size extension flag. (Set for enable)
#define X86_PAGE_GLOBAL	(1<<8)	// this flag is only used in page-table entries
// when a page is marked global and the page global enable (PGE) flag in register
// CR4 is set, the page-table or page-directory entry for the page is not invalidated
// in the TLB when register CR3 is loaded
#define X86_PAGE_AVAIL1	(1<<9)	// Available for usage by the Kernel
#define X86_PAGE_AVAIL2	(1<<10)	// Available for usage by the Kernel
#define X86_PAGE_AVAIL3	(1<<11) // Available for usage by the Kernel

#define PAGE_SIZE 4096