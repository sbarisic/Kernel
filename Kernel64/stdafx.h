#pragma once

#include <stdint.h>
//#include <intrin.h>

#include <string.h>

#define HAS_FLAG(n, f) (((n) & (f)) != 0)

#define KiB 0x400
#define MiB 0x100000
#define GiB 0x40000000


int gdt_code;

typedef uint32_t BOOL;
#define TRUE (1 == 1)
#define FALSE (1 != 1)

typedef struct {
	int32_t Size;
	uint64_t BaseAddr;
	uint64_t Len;
	uint32_t Type;
} MMAP_INFO;

typedef struct {
	uint32_t Len;
	uint32_t Addr;
} MULTIBOOT_INFO_MMAP;

extern void BREAKPOINT();