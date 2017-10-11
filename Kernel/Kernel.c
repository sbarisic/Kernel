#include "stdafx.h"

#define MULTIBOOT_DEFINE
#include "Kernel.h"

uint16_t*  VidMem;
int X;
int Y;

//uint32_t* PageTable;
//uint32_t* PageDirectory;

void write(const char* Str) {
	while (*Str) {
		VidMem[Y * 80 + X] = (uint16_t)(*Str) | 15 << 8;

		Str++;
		X++;
		if (X >= 80) {
			Y++;
			X = 0;
		}
	}
}

void writeln(const char* Str) {
	write(Str);
	X = 0;
	Y++;
}

void cls() {
	X = 0;
	Y = 0;
	for (int i = 0; i < 80 * 25; i++)
		VidMem[i] = 0;
}

#pragma function(memcpy)
void* __cdecl memcpy(void* Dst, void* Src, size_t Len) {
	for (size_t i = 0; i < Len; i++)
		((uint8_t*)Dst)[i] = ((uint8_t*)Src)[i];
}

uint64_t* PT4; // 512 GiB pages
uint64_t* PT3; // 1 GiB pages
uint64_t* PT2; // 2 MiB pages
uint64_t* PT1; // 4 KiB pages

void temp_paging_init() {
	// Put all tables at 0x1000 sequentially
	PT4 = (uint64_t*)0x1000;
	PT3 = (uint64_t*)0x2000;
	PT2 = (uint64_t*)0x3000;
	PT1 = (uint64_t*)0x4000;

	// Clear tables
	for (int i = 0; i < 512 * 4; i++)
		PT4[i] = NULL;

	// PT4[0] -> PT3[0] -> PT2
	PT4[0] = (uint64_t)PT3 | X86_PAGE_PRESENT | X86_PAGE_WRITE;
	PT3[0] = (uint64_t)PT2 | X86_PAGE_PRESENT | X86_PAGE_WRITE;

	// Start frame 0 with 2 MiB size
	uint64_t Addr = 0;
	uint64_t Sz = 0x200000;

	// Identity map first 32 MiB using 2 MiB pages
	for (int i = 0; i < 16; i++) {
		PT2[i] = Addr | X86_PAGE_PRESENT | X86_PAGE_WRITE | X86_PAGE_PAGESIZE;
		Addr += Sz;
	}

	__writecr3((uint32_t)PT4); // Shove the table into the CR3 register where it belongs
	__writecr4(__readcr4() | (1 << 5)); // Enable PAE

	const int MSR = 0xC0000080;
	__writemsr(MSR, __readmsr(MSR) | (1 << 8)); // Enable long mode

	__writecr0(__readcr0() | (1 << 31)); // Enable Paging
}

void full_halt() {
	_disable();
	while (1)
		__halt();
}

char* tohex(int val, char* str) {
	return itoa(val, str, 16);
}

void writenum(int b) {
	char* strr = (char*)0x1000;
	__stosb(strr, 0, 40);
	itoa(b, strr, 16);
	write(strr);
}

MULTIBOOT_INFO* Inf;

void kmain2() {
	VidMem = (uint16_t*)0xb8000;
	X = 0;
	Y = 0;

	cls();
	writeln("[kmain] Running in protected mode");

	uint64_t Kernel64Loc;
	uint64_t Kernel64Main;

	if ((Inf->Flags >> 3) & 1)
		for (int i = 0; i < Inf->ModsCount; i++) {
			if (strcmp(Inf->ModsAddr[i].String, "Kernel64") == 0) {
				uint32_t Start = Inf->ModsAddr[i].ModStart;
				uint32_t End = Inf->ModsAddr[i].ModEnd;
				uint32_t Len = End - Start;

				IMAGE_DOS_HEADER* DosHeader = (IMAGE_DOS_HEADER*)Start;
				if (DosHeader->e_magic == IMAGE_DOS_SIGNATURE) {

					PIMAGE_NT_HEADERS NTHeader = (PIMAGE_NT_HEADERS)(Start + DosHeader->e_lfanew);
					if (NTHeader->Signature == IMAGE_NT_SIGNATURE) {

						Kernel64Loc = NTHeader->OptionalHeader.ImageBase;
						memcpy(Kernel64Loc, Start, Len);

						Kernel64Main = Kernel64Loc + NTHeader->OptionalHeader.AddressOfEntryPoint;
					}
				}
			}
		}

	write("[kmain] Loaded Kernel64 to 0x");
	writenum(Kernel64Loc);
	write(", entry point at 0x");
	writenum(Kernel64Main);
	writeln("");

	temp_paging_init();
	writeln("[kmain] Temporary paging enabled, 32 MiB identity mapped");

	// TODO: GDC and far jump to Kernel64Main
}

__declspec(naked) void kmain() {
	__asm mov Inf, EBX;

	__asm mov ESP, (0x1000000 - 0x1000);
	__asm mov EBP, ESP;
	kmain2();
	full_halt();
}