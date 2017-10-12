#include "stdafx.h"

#define HALT _disable(); __halt()

#define MULTIBOOT_DEFINE
#include "Kernel.h"

MULTIBOOT_INFO* Inf;

uint16_t*  VidMem;
int X;
int Y;

uint64_t* PT4; // 512 GiB pages
uint64_t* PT3; // 1 GiB pages
uint64_t* PT2; // 2 MiB pages
uint64_t* PT1; // 4 KiB pages
uint64_t* GDT;

uint32_t Kernel64Loc;
uint32_t Kernel64Main;

//uint32_t* PageTable;
//uint32_t* PageDirectory;

void write(const char* Str) {
	__outbytestring(0xe9, Str, strlen(Str));

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
	__outbyte(0xe9, '\n');

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
	return Dst;
}

void temp_paging_init() {
	// Clear tables
	for (int i = 0; i < 512 * 4; i++)
		PT4[i] = 0;

	// PT4[0] -> PT3[0] -> PT2
	PT4[0] = (uint64_t)PT3 | X86_PAGE_PRESENT | X86_PAGE_WRITE;
	PT3[0] = (uint64_t)PT2 | X86_PAGE_PRESENT | X86_PAGE_WRITE;

	// Start frame 0 with 2 MiB size
	uint64_t Addr = 0;
	uint64_t Sz = 0x200000;

	// Identity map first 32 MiB using 2 MiB pages
	for (int i = 0; i < 64; i++) {
		PT2[i] = Addr | X86_PAGE_PRESENT | X86_PAGE_WRITE | X86_PAGE_PAGESIZE;
		Addr += Sz;
	}

	// Shove the table into the CR3 register where it belongs
	__writecr3((uint32_t)PT4);

	__writecr4(__readcr4() | (1LL << 5)); // Enable PAE

	const int MSR = 0xC0000080;
	__writemsr(MSR, __readmsr(MSR) | (1LL << 8)); // Enable long mode

	__writecr0(__readcr0() | (1LL << 31)); // Enable Paging
	//__writecr0(__readcr0() | (1LL << 16)); // Enable error on write thingie something.
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

void load_kernel64() {
	if ((Inf->Flags >> 3) & 1)
		for (int i = 0; i < (int)Inf->ModsCount; i++) {
			if (strcmp(Inf->ModsAddr[i].String, "Kernel64") == 0) {
				uint32_t Start = Inf->ModsAddr[i].ModStart;
				uint32_t End = Inf->ModsAddr[i].ModEnd;
				uint32_t Len = End - Start;

				IMAGE_DOS_HEADER* DosHeader = (IMAGE_DOS_HEADER*)Start;
				if (DosHeader->e_magic == IMAGE_DOS_SIGNATURE) {

					PIMAGE_NT_HEADERS NTHeader = (PIMAGE_NT_HEADERS)(Start + DosHeader->e_lfanew);
					if (NTHeader->Signature == IMAGE_NT_SIGNATURE) {

						Kernel64Loc = (uint32_t)NTHeader->OptionalHeader.ImageBase;
						memcpy((void*)Kernel64Loc, (void*)Start, Len);

						Kernel64Main = (uint32_t)(Kernel64Loc + (uint32_t)NTHeader->OptionalHeader.AddressOfEntryPoint);
					}
				}
			}
		}
}

struct {
	uint16_t Size;
	uint32_t Base;
}  LGDTRPtr;

void kmain2() {
	_disable();
	VidMem = (uint16_t*)0xb8000;

	// Put all tables at 0x1000 sequentially
	uint64_t Base = 0x2000000;
	PT4 = (uint64_t*)(Base);
	PT3 = (uint64_t*)(Base + 0x1000);
	PT2 = (uint64_t*)(Base + 0x2000);
	PT1 = (uint64_t*)(Base + 0x3000);
	GDT = (uint64_t*)(Base + 0x4000);

	X = 0;
	Y = 0;

	cls();
	write("[kmain] Protected mode, base at 0x");
	writenum((uint32_t)&Hdr);
	writeln("");
	BREAK;

	temp_paging_init();
	writeln("[kmain] Temporary paging, 128 MiB identity mapped");
	BREAK;

	load_kernel64();
	write("[kmain] Loaded Kernel64 to 0x");
	writenum(Kernel64Loc);
	write(", entry point at 0x");
	writenum((uint32_t)Kernel64Main);
	writeln("");
	BREAK;

#define F(n) (1LL << n)

	GDT[0] = 0;
	//GDT[1] = F(43) | F(44) | F(47) | F(53) | F(41);  // code
	//GDT[2] = F(44) | F(47) | F(41); // data

	GDT[1] = 0x0020980000000000 /*| F(41)*/;
	GDT[2] = 0x0020920000000000;

	//GDT[1] = (1LL << 44) | (1LL << 47) | (1LL << 41) | (1LL << 43) | (1LL << 53);
	//GDT[2] = (1LL << 44) | (1LL << 47) | (1LL << 41);

	GDT[3] = 0;

	LGDTRPtr.Size = sizeof(uint64_t) * 3 - 1;
	LGDTRPtr.Base = (uint32_t)GDT;
	_lgdt(&LGDTRPtr);

	BREAK;

	_asm {
		mov ax, 0
		mov fs, ax

		mov ax, 16
		mov ss, ax
		mov ds, ax
		mov es, ax

		mov ax, 8
		mov gs, ax
	};

	BREAK;

	__asm {
		jmp gs : Kernel64Main
	};
}

__declspec(naked) void kmain() {
	__asm mov Inf, EBX;

	__asm mov ESP, (0x1000000 - 0x100);
	__asm mov EBP, ESP;
	kmain2();
	HALT;
}