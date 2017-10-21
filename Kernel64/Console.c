#include "stdafx.h"
#include "Console.h"
#include "itoa_64.h"

//#include <varargs.h>

int ConW;
int ConH;

int ConX;
int ConY;

uint16_t* VidMem = (uint16_t*)0xb8000;

void console_put(char C) {
	__outbyte(0xE9, (unsigned char)C);

	// Put
	int Idx = ConY * ConW + ConX;
	if (C != '\n')
		VidMem[Idx] = ((uint16_t)C) | 0x0F00;

	ConX++;
	if (ConX >= ConW || C == '\n') {
		ConX = 0;
		ConY++;
	}

	// Scroll
	if (ConY >= ConH) {
		ConY = ConH - 1;

		for (int y = 1; y < ConH; y++) {
			for (int x = 0; x < ConW; x++) {
				VidMem[(y - 1) * ConW + x] = VidMem[y * ConW + x];
				VidMem[y * ConW + x] = 0;
			}
		}
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

void console_writedec(int32_t Num) {
	char buf[1024] = { 0 };
	itoa_64(Num, buf, 10);
	console_write(buf);
}

void console_init(int32_t X, int32_t Y) {
	ConX = X;
	ConY = Y;
	ConW = 80;
	ConH = 25;

	/*for (int i = 0; i < ConW * ConH; i++)
		console_put_legacy(i, ' ');*/
}