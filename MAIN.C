#include <conio.h>
#include <stdio.h>
#include <dos.h>
#include "sb.h"
#include "timer.h"

static PATCH op_sine = {{0x00, 0x21}, {0x3F, 0x00}, {0x00, 0xFF},
		       {0x00, 0x0F}, {0x00, 0x00}, 0x00};

void setscreen(void);
void drawscreen(void);

void drawscreen(void)
{
	char far *screenptr;
	int i;

	screenptr = MK_FP(0xB800, 0);

	for (i = 1; i < 79; i++) {
		*(screenptr + i*2) = 'Í';
		*(screenptr + 24*160 + i*2) = 'Í';
	}
	for (i = 1; i < 24; i++) {
		*(screenptr + i*160) = 'º';
		*(screenptr + i*160 + 158) = 'º';
	}
	*screenptr = 'É';
	*(screenptr + 158) = '»';
	*(screenptr + 24*160) = 'È';
	*(screenptr + 24*160 + 158) = '¼';

	for (i = 1; i < 9; i++)
	{
		*(screenptr + 160 + 30+i*12) = 0x30 + i;
		*(screenptr + 160 + 30+i*12+1) = 3;
	}

	getch();
}

void setscreen(void)
{
	asm {
		mov ax, 3h
		int 10h
	}
}

int main(int argc, char *argv[], char *envp[])
{
	int opl_type;

	(void)argc;
	(void)argv;
	(void)envp;

	if (!(opl_type = sb_detect())) {
		cprintf("No soundblaster found. Check your BLASTER env setting.\n");
		return -1;
	}

	switch(opl_type) {
	case 1:
		cprintf("\r\nOPL2 found at 0x388\r\n");
		break;
	default:
		cprintf("Unknown soundcard.\r\n");
		return -2;
	}

	timer_install(1193);	// should produce 1000 Hz

	opl_reset();
	opl_patch(0, &op_sine);
	opl_patch(1, &op_sine);
	opl_patch(2, &op_sine);
	//opl_rythm_mode();
	//opl_percussion_on(PERC_BASE, 270);
/*	while(!kbhit()) {
		opl_tone_f(0, 100 + rand()%1000, 63);
		opl_tone_f(1, 100 + rand()%1000, 63);
		opl_tone_f(2, 100 + rand()%1000, 63);
		delay_int8(rand() % 500);
	}
*/
	// gleichstufige Stimmung
	opl_tone_f(0, 262, 63);	 // c
	opl_tone_f(1, 330, 63);	 // e
	opl_tone_f(2, 392, 63);	 // g
	delay_int8(1000);
	// reine Stimmung
	opl_tone_f(0, 264, 63);	 // c
	opl_tone_f(1, 330, 63);	 // e
	opl_tone_f(2, 396, 63);	 // g
	delay_int8(1000);

	// gleichstufig moll
	opl_tone_f(0, 264, 63);	 // c
	opl_tone_f(1, 311, 63);	 // e
	opl_tone_f(2, 396, 63);	 // g


	delay_int8(1000);

	opl_reset();

	timer_restore();
	cprintf("old int 8 calls: %lu\r\n", defaulthandlercalls);
	return 0;

	setscreen();
	drawscreen();
	return 0;
}
