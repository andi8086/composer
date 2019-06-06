#include "sb.h"
#include <dos.h>

static BYTE fm_car_ksl[8];	// carrier ksl
static BYTE fm_mod_chars[8];    // modulator
static BYTE fm_car_chars[8];    // carrier w/ mult
static BYTE fm_rB0[8];          // high byte of block/f

void opl_set_reg(int reg, BYTE val)
{
	int i;
	outp(OPL_ADDR, reg);
	for (i = 0; i < 6; i++) inp(OPL_ADDR);	// 3.3 us delay
	outp(OPL_DATA, val);
	for (i = 0; i < 35; i++) inp(OPL_ADDR); // 23 us delay
}

int sb_detect(void)
{
	int i;
	BYTE status1, status2;
	int res;

	res = 0;

	opl_set_reg(4, 0x60);
	opl_set_reg(4, 0x80);
	status1 = inp(OPL_STATUS);

	opl_set_reg(2, 0xFF);
	opl_set_reg(4, 0x21);

	for (i = 0; i < 125; i++)
		inp(OPL_ADDR);

	status2 = inp(OPL_STATUS);

	opl_set_reg(4, 0x60);
	opl_set_reg(4, 0x80);

	if (status1 & 0xE0 != 0) return 0;
	if (status2 & 0xE0 != 0xC0) return 0;

	// we have found at least OPL2
	res = 1;

	return res;
}

void opl_reset(void)
{
	int i;

	for (i = 1; i <= 0xF5; i++)
		opl_set_reg(i, 0x00);

	opl_set_reg(0x01, 0x20);	// enable wave forms
}

void opl_ch_reg(int reg_base, int ch, BYTE val)
{
	int reg = reg_base + ch;

	opl_set_reg(reg, val);
}

void opl_op_reg(int reg_base, int ch, int op, BYTE val)
{
	int reg = reg_base + ch;
	if (ch > 2) reg += 5;
	if (ch > 5) reg += 5;
	if (op)	    reg += 3;

	opl_set_reg(reg, val);
}

void opl_patch(int ch, PPTR_PATCH patch)
{
	opl_op_reg(0x20, ch, 0, patch->chars[0]);
	opl_op_reg(0x20, ch, 1, patch->chars[1]);
	opl_op_reg(0x40, ch, 0, patch->ksl_lev[0]);
	opl_op_reg(0x40, ch, 1, patch->ksl_lev[1]);
	opl_op_reg(0x60, ch, 0, patch->att_dec[0]);
	opl_op_reg(0x60, ch, 1, patch->att_dec[1]);
	opl_op_reg(0x80, ch, 0, patch->sus_rel[0]);
	opl_op_reg(0x80, ch, 1, patch->sus_rel[1]);
	opl_op_reg(0xE0, ch, 0, patch->wav_sel[0]);
	opl_op_reg(0xE0, ch, 1, patch->wav_sel[1]);
	opl_ch_reg(0xC0, ch,	patch->fb_conn);

	fm_car_ksl[ch] = patch->ksl_lev[1];
	fm_mod_chars[ch] = patch->chars[0];
	fm_car_chars[ch] = patch->chars[1];
}

static WORD opl_get_block_fnum(int freq)
{
	DWORD f_num = 65536UL * freq / 3125;
	WORD block = 0;

	while ((f_num > 0x3FF) && (block < 8))
	{
		block += 1;
		f_num >>= 1;
	}
	if (block > 7) return 0xFFFF;
	return (block << 10) | (WORD) f_num;
}

void opl_tone_f(int ch, int freq, BYTE vol)
{
	WORD block_fnum;
	int mult = 1;
	int c_mult, m_mult;

	BYTE r20m, r20c, r40c, rA0, rB0;
	do {
		block_fnum = opl_get_block_fnum(freq/mult);
		if (block_fnum == 0xFFFF)
		{
			mult <<=1;
		}
		if (mult == 16) mult = 15; else
		if (mult == 15) break;
	} while (block_fnum == 0xFFFF);

	m_mult = (fm_mod_chars[ch] & 0x0F) * mult;
	c_mult = (fm_car_chars[ch] & 0x0F) * mult;

	if ((block_fnum == 0xFFFF) || (m_mult > 15)
	    || (c_mult > 15)) return;

	r20m = (fm_mod_chars[ch] & 0xF0) | m_mult;
	r20c = (fm_car_chars[ch] & 0xF0) | c_mult;

	r40c = (fm_car_ksl[ch] & 0xC0) | (63-vol);

	block_fnum |= 0x2000;

	rA0 = block_fnum & 0xFF;
	rB0 = block_fnum >> 8;

	fm_rB0[ch] = rB0;
	opl_op_reg(0x20, ch, 0, r20m);
	opl_op_reg(0x20, ch, 1, r20c);
	opl_op_reg(0x40, ch, 1, r40c);

	opl_ch_reg(0xA0, ch, rA0);
	opl_ch_reg(0xB0, ch, rB0);
}

void opl_rythm_mode(void)
{
	opl_set_reg(0xBD, 0x20);
}

void opl_percussions_off(void)
{
	opl_set_reg(0xBD, 0x00);
}

void opl_percussion_on(BYTE val, int freq)
{
	val &= 0x1F;
	val |= 0x20;
	opl_ch_reg(0xA0, 6, freq & 0xFF);
	opl_ch_reg(0xB0, 6, (freq >> 8) & 0x1F);
	opl_op_reg(0x60, 6, 0, 0xFF);
	opl_op_reg(0x60, 6, 0, 0xFF);
	opl_op_reg(0x80, 6, 1, 0xA2);
	opl_op_reg(0x80, 6, 1, 0xA2);
	opl_op_reg(0x40, 6, 0, 0x08);
	opl_op_reg(0x40, 6, 1, 0x08);
	opl_op_reg(0x20, 6, 0, 0x01);
	opl_op_reg(0x20, 6, 1, 0x01);


	opl_set_reg(0xBD, val);
}

void opl_melodic_mode(void)
{
	opl_set_reg(0xBD, 0);
}
