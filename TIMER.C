#include <dos.h>
#include "timer.h"

volatile unsigned short tickcount;
volatile unsigned long intcount;
volatile unsigned short divisor;
volatile unsigned long defaulthandlercalls;

void interrupt (*default_handler)(void);

void interrupt IRQ0_handler(void)
{
	intcount++;
	asm {
		mov ax, divisor
		add tickcount, ax
		jnc skip_default_handler
		int 0E0h
	}
	defaulthandlercalls++;
	goto IRQ0_exit;
skip_default_handler:
	asm {
		mov al, 20h
		out 20h, al
	}
IRQ0_exit:
}

void timer_install(unsigned short div)
{
	divisor = div;
	tickcount = 0;
	intcount = 0;
	defaulthandlercalls = 0;
	default_handler = getvect(0x08);
	setvect(0xE0, default_handler);
	setvect(0x08, IRQ0_handler);
	asm {
		// counter 0, low-byte + high-byte, square-wave, no BCD
		mov al, 00110110b
		out 43h, al
		mov al, byte ptr [divisor]
		out 40h, al
		mov al, byte ptr [divisor + 1]
		out 40h, al
	}
}

void timer_restore(void)
{
	asm {
		// counter 0, low-byte + high-byte, square-wave, no BCD
		mov al, 00110110b
		out 43h, al
		mov al, 0xFF
		out 40h, al
		mov al, 0xFF
		out 40h, al
	}
	setvect(0x08, default_handler);
}

void delay_int8(unsigned long count)
{
	asm cli;
	intcount = 0;
	asm sti;
	while(intcount < count) asm nop;
}