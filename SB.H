#ifndef __SB_H__
#define __SB_H__

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;

#define OPL_ADDR	0x388
#define OPL_STATUS	0x388
#define OPL_DATA	0x389

#define PERC_BASE 	0x10
#define	PERC_SNARE      0x08
#define PERC_TOMTOM     0x04
#define PERC_CYMBAL     0x02
#define PERC_HIHAT      0x01

typedef struct {
	BYTE chars[2];
	BYTE ksl_lev[2];
	BYTE att_dec[2];
	BYTE sus_rel[2];
	BYTE wav_sel[2];
	BYTE fb_conn;
} PATCH;

typedef far PATCH far *PPTR_PATCH;

int sb_detect(void);
void opl_reset(void);
void opl_ch_reg(int reg_base, int ch, BYTE val);
void opl_op_reg(int reg_base, int ch, int op, BYTE val);
void opl_patch(int ch, PPTR_PATCH patch);
void opl_tone_f(int ch, int freq, BYTE vol);
void opl_rythm_mode(void);
void opl_percussion_on(BYTE val, int freq);
void opl_melodic_mode(void);
void opl_percussions_off(void);

#endif