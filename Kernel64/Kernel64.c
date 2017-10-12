#include "stdafx.h"

#include <stdint.h>
#include <intrin.h>

uint16_t* VidMem;

void write(const char* Str) {
	int i = 0;

	while (*Str) 
		VidMem[i++] = (uint16_t)(*Str) | 15 << 8;
}

void __cdecl main() {
	//VidMem = (uint16_t*)0xb8000;

	//write("LONG MODE BABY");

	_disable();
	__halt();
}

