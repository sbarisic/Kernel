#include "stdafx.h"
#include "Console.h"
#include "itoa_64.h"

#include <varargs.h>

int ConW;
int ConH;

int ConX;
int ConY;

inline void console_put_legacy(int Idx, char C) {
	if (C != '\n') {
		uint16_t* VidMem = (uint16_t*)0xb8000;
		VidMem[Idx] = ((uint16_t)C) | 0x0F00;
	}
}

void console_put(char C) {
	__outbyte(0xE9, (unsigned char)C);
	console_put_legacy(ConY * ConW + ConX, C);

	ConX++;
	if (ConX >= ConW || C == '\n') {
		ConX = 0;
		ConY++;
	}
}

void console_write(const char* Str) {
	size_t Len = strlen(Str);

	for (size_t i = 0; i < Len; i++)
		console_put(Str[i]);
}

void console_writehex(int32_t Num) {
	char buf[1024] = { 0 };
	itoa_64(Num, buf, 16);
	console_write("0x");
	console_write(buf);
}

void console_init() {
	ConX = 0;
	ConY = 0;
	ConW = 80;
	ConH = 25;

	for (int i = 0; i < ConW * ConH; i++)
		console_put_legacy(i, ' ');
}