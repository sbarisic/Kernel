#include "stdafx.h"

#define HALT _disable(); __halt()

#include "Kernel.h"

const char* ServiceNames[] = {
	"$PnP",
	"$PCI",
	"$PIR",
	"$PMM",
	"$BC$",
	"$ACF",
	"$BLK",
	"$WDS",
	"$SNY",
	"$SDS",
	"$SDM",
	"$IRT",
	"$CFD",
	"$BBS",
	"$ASF",
	"$$CT",
	"MPNT",
	"PCI "
};

PMInfoBlock* PMInfo;
BIOS32* B32;

MULTIBOOT_INFO* MultibootInfo;
uint64_t* PT4; // 512 GiB pages
uint64_t* PT3; // 1 GiB pages
uint64_t* PT2; // 2 MiB pages
uint64_t* PT1; // 4 KiB pages
//uint64_t* GDT;

uint16_t*  VidMem;
int X;
int Y;

uint32_t Kernel64Loc;
uint32_t Kernel64Main;

//uint32_t* PageTable;
//uint32_t* PageDirectory;

void write(const char* Str) {
	__outbytestring(0xe9, (unsigned char*)Str, strlen(Str));

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

	__writecr4(__readcr4() | (1UL << 5)); // Enable PAE

	const unsigned int MSR = 0xC0000080;
	__writemsr(MSR, __readmsr(MSR) | (1UL << 8)); // Enable long mode

	__writecr0(__readcr0() | (1UL << 31)); // Enable Paging
}

void writenum(int b) {
	char* strr = (char*)0x1000;
	__stosb(strr, 0, 40);
	itoa_32(b, strr, 16);
	write(strr);
}

void load_kernel64() {
	if ((MultibootInfo->Flags >> 3) & 1)
		for (int i = 0; i < (int)MultibootInfo->ModsCount; i++) {
			if (strcmp(MultibootInfo->ModsAddr[i].String, "Kernel64") == 0) {
				uint32_t Start = MultibootInfo->ModsAddr[i].ModStart;
				uint32_t End = MultibootInfo->ModsAddr[i].ModEnd;
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

uint64_t align(uint64_t Num, uint64_t RoundNum) {
	return (Num + RoundNum - 1) & ~(RoundNum - 1);
}

int check_bios32(BIOS32* B32, const char* Serv) {
	uint32_t Entry = B32->Entry;
	uint8_t Exists = 0;
	uint32_t Name = Serv[0] | (Serv[1] << 8) | (Serv[2] << 16) | (Serv[3] << 24);

	__asm {
		mov bl, 0
		mov eax, Name
		mov ebx, 0

		push cs
		push retloc
		push cs
		push Entry
		retf
		retloc :
		mov Exists, al
			mov al, 0
	}

	return Exists == 0;
}

PMInfoBlock* find_vbe_info(uint32_t Offset) {
	for (int i = 0; i < (1 << 16); i++) {
		PMInfoBlock* Block = (PMInfoBlock*)(Offset + i);

		if (Block->Sig == 'PMID' || Block->Sig == 'DIMP')
			return Block;
	}

	return NULL;
}

struct {
	int32_t ConX;
	int32_t ConY;
	MULTIBOOT_INFO* Multiboot;
} Data = { 0 };

void kmain() {
	VidMem = (uint16_t*)0xb8000;

	PT4 = (uint64_t*)align((uint64_t)PT4 + 0x1000 * 0, 0x1000);
	PT3 = (uint64_t*)align((uint64_t)PT3 + 0x1000 * 1, 0x1000);
	PT2 = (uint64_t*)align((uint64_t)PT2 + 0x1000 * 2, 0x1000);
	PT1 = (uint64_t*)align((uint64_t)PT1 + 0x1000 * 3, 0x1000);
	//GDT = (uint64_t*)align(GDT + 0x1000 * 4, 0x1000);

	/*// Put all tables at 0x1000 sequentially
	uint64_t Base = 0x2000000;
	PT4 = (uint64_t*)(Base);
	PT3 = (uint64_t*)(Base + 0x1000);
	PT2 = (uint64_t*)(Base + 0x2000);
	PT1 = (uint64_t*)(Base + 0x3000);
	GDT = (uint64_t*)(Base + 0x4000);*/

	X = 0;
	Y = 0;

	cls();

	//char CPUName[0x40] = { 0 };
	char CPUName[0x40];

	int CPUInfo[4] = { -1 };
	__cpuid(CPUInfo, 0x80000000);
	uint32_t NexIDs = CPUInfo[0];

	for (uint32_t i = 0x80000000; i <= NexIDs; ++i) {
		__cpuid(CPUInfo, i);
		if (i == 0x80000002)
			memcpy(CPUName, CPUInfo, sizeof(CPUInfo));
		else if (i == 0x80000003)
			memcpy(CPUName + 16, CPUInfo, sizeof(CPUInfo));
		else if (i == 0x80000004)
			memcpy(CPUName + 32, CPUInfo, sizeof(CPUInfo));
	}

	int SpaceIdx = 0;
	while (CPUName[SpaceIdx] == ' ')
		SpaceIdx++;
	writeln(CPUName);

	write("[kmain] Protected mode, base at 0x");
	writenum((uint32_t)&multiboot_header);
	writeln("");

	write("[kmain] Searching for VBE 3 Info block");

	PMInfo = find_vbe_info(0xC0000);
	if (PMInfo == NULL) {
		PMInfo = find_vbe_info(0xC000);

		if (PMInfo == NULL) {
			PMInfo = find_vbe_info(0x0);
		}
	}

	if (PMInfo == NULL)
		writeln(" ... FAIL");
	else
		writeln(" ... OKAY");

	temp_paging_init();
	writeln("[kmain] Temporary paging, 128 MiB identity mapped");

	load_kernel64();
	write("[kmain] Loaded Kernel64 to 0x");
	writenum(Kernel64Loc);
	write(", entry point at 0x");
	writenum((uint32_t)Kernel64Main);
	writeln("");

	write("[kmain] Searching for BIOS32");

	uint8_t* Start = (uint8_t*)0xE0000;
	for (; Start <= (uint8_t*)0xFFFF0; Start += 16) {
		B32 = NULL;

		if ((B32 = (BIOS32*)Start)->Sig == '_23_') {
			writeln(" ... OKAY");
			writeln("[kmain] Scanning BIOS32 services");

			for (int i = 0; i < (sizeof(ServiceNames) / sizeof(*ServiceNames)); i++) {
				if (check_bios32(B32, ServiceNames[i])) {
					write(ServiceNames[i]);
					write(" ");
				}
			}
			break;
		}
		else
			B32 = NULL;
	}

	if (B32 == NULL)
		write("... FAIL");
	writeln("");


	Data.ConX = X;
	Data.ConY = Y;
	Data.Multiboot = MultibootInfo;
	enter_long();
}