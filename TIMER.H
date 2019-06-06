#ifndef __TIMER_H__
#define __TIMER_H__

extern volatile unsigned long intcount;
extern volatile unsigned long defaulthandlercalls;

void timer_install(unsigned short div);
void timer_restore(void);
void delay_int8(unsigned long count);


#endif
