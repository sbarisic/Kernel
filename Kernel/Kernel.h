#pragma once

#define KERNEL_STACK 0x0
#define STACK_SIZE 0x4000

#define MULTIBOOT_HEADER_MAGIC 0x1BADB002
#define CHECKSUM -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)
#define MULTIBOOT_HEADER_FLAGS ((1<<16) | (1<<1) | (1<<0) /*| (1<<2)*/) // uncomment to enable gfx

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

void kmain();

#ifdef MULTIBOOT_DEFINE
#pragma section(".text")
__declspec(allocate(".text")) MULTIBOOT_HEADER Hdr = {
	MULTIBOOT_HEADER_MAGIC,
	MULTIBOOT_HEADER_FLAGS,
	CHECKSUM,

	&Hdr,
	&Hdr,
	0,
	0,
	&kmain,

	0,
	800,
	600,
	32,
};
#endif