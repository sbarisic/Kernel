#include "stdafx.h"
#include "Interrupts.h"
#include "Console.h"

const char* IRQs[] = {
	"Programmable Interrupt Timer",
	"Keyboard Interrupt",
	"Cascade",
	"COM2",
	"COM1",
	"LPT2",
	"Floppy Disk",
	"LPT1 / Spurious",
	"CMOS RTC",
	"Free 9",
	"Free 10",
	"Free 11",
	"PS2 Mouse",
	"Coprocessor",
	"Primary ATA HD",
	"Secondary ATA HD"
};

const char* Exceptions[] = {
	"Division by zero",
	"Single-step interrupt",
	"NMI",
	"Breakpoint",
	"Overflow",
	"Bounds",
	"Invalid opcode",
	"Coprocessor not available",
	"Double fault",
	"Coprocessor segment overrun",
	"Invalid TSS",
	"Segment not present",
	"Stack fault",
	"General protection fault",
	"Page fault",
	"Reserved",
	"Math fault",
	"Alignment check",
	"Machine check",
	"SIMD FP exception",
	"Virtualization exception",
	"Control protection exception"
};

typedef struct {
	uint16_t LoOffset;
	uint16_t Selector;

	uint8_t IST;
	uint8_t Flags;

	uint16_t MiOffset;
	uint32_t HiOffset;
	uint32_t Zero2;
} InterruptDescriptor;

InterruptDescriptor InterruptTable[255] = { 0 };

struct {
	uint16_t Limit;
	uint64_t Base;
} InterruptTablePtr = { 0 };

void int_init_descriptor(uint32_t Entry, uint16_t Sel, uint64_t Handler, uint8_t Flags) {
	InterruptDescriptor* E = &(InterruptTable[Entry]);

	E->LoOffset = Handler & 0xFFFF;
	E->MiOffset = (Handler >> 16) & 0xFFFF;
	E->HiOffset = Handler >> 32;

	E->Selector = Sel;

	E->Zero2 = 0;
	E->IST = 0;

	//E->Flags = INT_FLAG_PRESENT | INT_FLAG_PRIV_0 | INT_FLAG_INTGATE;
	E->Flags = Flags;

	//InterruptTable[Entry].Selector = Sel;
	//InterruptTable[Entry].Type = Type;
	//InterruptTable[Entry].LoOffset = (Handler & 0xFFFFFFFF);
	//InterruptTable[Entry].HiOffset = (Handler >> 32);
}

void IRQInit(uint8_t MasterOffset, uint8_t SlaveOffset) {
	// Init PICs
	__outbyte(0x20, 0x11);
	__outbyte(0xA0, 0x11);

	// Master vector offset
	__outbyte(0x21, MasterOffset);
	// Slave vector offset
	__outbyte(0xA1, SlaveOffset);

	// Tell master there is slave
	__outbyte(0x21, 0x04);
	// Tell slave cascade identity
	__outbyte(0xA1, 0x02);

	__outbyte(0x21, 0x01);
	__outbyte(0xA1, 0x01);

	// Set masks to 0
	__outbyte(0x21, 0x0);
	__outbyte(0xA1, 0x0);
}

void IRQMask(uint8_t Line, int32_t Set) {
	uint16_t Port;

	if (Line < 8)
		Port = 0x21;
	else {
		Port = 0xA1;
		Line -= 8;
	}

	unsigned char Val;
	if (Set)
		Val = __inbyte(Port) | (1 << Line);
	else
		Val = __inbyte(Port) & ~(1 << Line);

	__outbyte(Port, Val);
}

void init_interrupts() {
	_disable();
	
	InterruptTablePtr.Limit = sizeof(InterruptTable) - 1;
	InterruptTablePtr.Base = (uint64_t)&InterruptTable;
	for (uint32_t i = 0; i < (sizeof(InterruptTable) / sizeof(*InterruptTable)); i++)
		int_init_descriptor(i, 0, 0, 0);

#define INT_INIT_NUM(n) void int_handler_ ## n (); int_init_descriptor(n, gdt_code, (uint64_t) &int_handler_ ## n, INT_FLAG_PRESENT | INT_FLAG_PRIV_0 | INT_FLAG_INTGATE)
	INT_INIT_NUM(0);
	INT_INIT_NUM(1);
	INT_INIT_NUM(2);
	INT_INIT_NUM(3);
	INT_INIT_NUM(4);
	INT_INIT_NUM(5);
	INT_INIT_NUM(6);
	INT_INIT_NUM(7);
	INT_INIT_NUM(8);
	INT_INIT_NUM(9);
	INT_INIT_NUM(10);
	INT_INIT_NUM(11);
	INT_INIT_NUM(12);
	INT_INIT_NUM(13);
	INT_INIT_NUM(14);
	INT_INIT_NUM(15);
#undef INT_INIT_NUM

	int_init_descriptor(80, gdt_code, (uint64_t)&int_handler, INT_FLAG_PRESENT | INT_FLAG_PRIV_0 | INT_FLAG_INTGATE);


	IRQInit(32, 40);
	for (uint8_t i = 0; i < 16; i++)
		IRQMask(i, 1);

	__lidt(&InterruptTablePtr);
	_enable();

	TRACELN("Interrupts enabled");
}

void int_handler2(int32_t Error, int32_t IntNum) {
	if (IntNum < 16) {
		console_write("[EXCEPTION ");
		console_writehex(IntNum);
		console_write("] ");
		console_write(Exceptions[IntNum]);
		console_write(" ");
		console_writehex(Error);
		console_write("\n");

		while (1)
			;
	}

	console_write("[INTERRUPT ");
	console_writehex(IntNum);
	console_write("]\n");
}