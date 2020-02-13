/*
 * Copyright (C) 2013,2016 Urja Rannikko <urjaman@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

// CONSOLE SUBSYSTEM
#include "main.h"
#include "uart.h"
#include "console.h"
#include "ciface.h"
#include "lib.h"

#define BEL 0x07
#define BS 0x08
#define TAB 0x09
#define LF 0x0A
#define CR 0x0D
#define ESC 0x1B
#define DEL 0x7F
#define SPACE 0x20

#define utf8_cont(x) ((x & 0xC0) == 0x80)
#define utf8_pfx(x) ((x >= 0xC2) && (x <= 0xF4))
#define utf8_pfx4(x) ((x & 0xF8) == 0xF0)
#define utf8_pfx3(x) ((x & 0xF0) == 0xE0)
#define utf8_pfx2(x) ((x & 0xE0) == 0xC0)

static uint8_t gl_detect_invalid(uint8_t val, uint8_t u8s)
{
	if ( (val < 32) && ( val != CR && val != LF && val != BS && val != TAB && val != BEL && val != ESC ) )
		return 1; // exit on unknown control char (=binary protocol format)

	if ((!u8s) && utf8_cont(val)) /* UTF-8 continuation without prefix */
		return 1;

	/* Invalid UTF-8 prefix codes - we pass 0xFF for TELNET options for other processing */
	if ((val == 0xC0) || (val == 0xC1) || ((val >= 0xF5) && (val < 0xFF)))
		return 1;

	return 0;
}

static uint8_t gl_pfx_u8s(uint8_t val) {
	if (utf8_pfx(val)) {
		if (utf8_pfx4(val)) return 3;
		else if (utf8_pfx3(val)) return 2;
		else return 1;
	}
	return 0;
}

static uint8_t gl_parse_u8s(uint8_t val, uint8_t u8s) {
	if (u8s) {
		if (utf8_cont(val)) u8s--;
		else u8s = 0;
	}

	if (utf8_pfx(val)) {
		u8s = gl_pfx_u8s(val);
	}

	return u8s;
}

static uint8_t gl_do_bs(uint8_t *buf, uint8_t i) {
	ciface_send(BS);
	ciface_send(SPACE);
	ciface_send(BS);
	/* How many UTF-8 continuation bytes to wipe from the buf while at it */
	uint8_t u8cs = 0;
	for (uint8_t nbs = 0; nbs < (i-1); nbs++) {
		if (!utf8_cont(buf[i - (nbs+1)])) break;
		if (nbs >= 3) break;
		u8cs = nbs+1;
	}
	i = i - (2 + u8cs);
	return i;
}

uint8_t getline(unsigned char *buf, unsigned char len)
{
	unsigned char val,i;
	uint8_t u8s = 0; /* Expect N UTF-8 continuation characters */
	memset(buf,0,len);
	for(i=0; i<len; i++) {
#ifdef ciface_peek
		val = ciface_peek();
#else
		val = ciface_recv();
#endif
		if (gl_detect_invalid(val, u8s))
			return 1;

#ifdef ciface_peek
		val = ciface_recv();
#endif
		u8s = gl_parse_u8s(val, u8s);

		if (((val == BS)||(val == DEL))&&(i)) { // BS/DEL (both as backspace) processing
			i = gl_do_bs(buf, i);
			continue;
		}
		if (val == CR) { ciface_send(CR); ciface_send(LF); buf[i] = 0; break; } // Understand LF
		if ((val < 32)||(val == DEL)) { i--; continue; }
		if (val == 255) { ciface_recv(); ciface_recv(); i--; continue; } // Filter TELNET options
		buf[i] = val;
		ciface_send(val);
	}
	buf[len-1] = 0;
	return 0;
}

static uint8_t recover_u8s(unsigned char *buf, unsigned char i) {
	uint8_t u8s = 0;
	if (!i) return u8s;
	if (!(buf[i-1] & 0x80)) return u8s;
	uint8_t lkb;
	uint8_t lko;
	for (lkb = 0; lkb < 3; lkb++) {
		lko = i - (lkb+1);
		if (!lko) break; // If the byte we're about to look is the last byte we could look back to,
				// it has to be the prefix byte, so no need to check it for continuation, just end.
				// (we do check that the lookback byte is a prefix byte later)
		if (utf8_cont(buf[lko]))
			continue;
		break;
	}
	u8s = gl_pfx_u8s(buf[lko]);

	if (lkb > u8s) u8s = 0;
	else u8s = u8s - lkb;

	return u8s;
}

uint8_t getline_mc(unsigned char *buf, unsigned char len)
{
	unsigned char val,i;
	if (!ciface_isdata()) {
		return 0;
	}
	uint8_t u8s = recover_u8s(buf, getline_i);
	for(i=getline_i; i<len; i++) {
		if (ciface_isdata()) {
			val = ciface_recv();
			if (gl_detect_invalid(val, u8s))
				continue;
			u8s = gl_parse_u8s(val, u8s);

			if (((val == BS)||(val == DEL))&&(i)) { // BS/DEL (both as backspace) processing
				i = gl_do_bs(buf, i);
				continue;
			}

			if (val == CR) { ciface_send(CR); ciface_send(LF); buf[i] = 0; break; }; // Understand LF
			if ((val < 32)||(val == DEL)) { i--; continue; };
			if (val == 255) { ciface_recv(); ciface_recv(); i--; continue; }; // Filter TELNET options
			buf[i] = val;
			ciface_send(val);
		} else {
			getline_i = i;
			return 0;
		}
	}
	buf[len-1] = 0;
	getline_i = 0;
	return 1;
}


void sendstr_P(PGM_P str)
{
	unsigned char val;
	for(;;) {
		val = pgm_read_byte(str);
		if (val) ciface_send(val);
		else break;
		str++;
	}
}

void sendstr(const unsigned char * str)
{
	unsigned char val;
	for(;;) {
		val = *str;
		if (val) ciface_send(val);
		else break;
		str++;
	}
}

void sendcrlf(void)
{
	sendstr_P(PSTR("\r\n"));
}

void u32outdec(unsigned long int val)
{
	unsigned char buf[11];
	luint2str(buf,val);
	sendstr(buf);
}

void luint2outdual(unsigned long int val)
{
	unsigned char buf[11];
	luint2str(buf,val);
	sendstr(buf);
	sendstr_P(PSTR(" (0x"));
	luint2xstr(buf,val);
	sendstr(buf);
	sendstr_P(PSTR(") "));
}

unsigned char* scanfor_notspace(unsigned char *buf)
{
	for (;;) {
		if (!(*buf)) return buf;
		if (!isspace(*buf)) return buf;
		buf++;
	}
}

unsigned char* scanfor_space(unsigned char *buf)
{
	for (;;) {
		if (!(*buf)) return buf;
		if (isspace(*buf)) return buf;
		buf++;
	}
}

static unsigned char count_tokens(unsigned char *rcvbuf)
{
	unsigned char tokens=0;
	for (;;) {
		rcvbuf = scanfor_notspace(rcvbuf);
		if(!(*rcvbuf)) break;
		tokens++;
		rcvbuf = scanfor_space(rcvbuf);
		if (!(*rcvbuf)) break;
	}
	return tokens;
}

uint8_t tokenize(unsigned char *rcvbuf,unsigned char** ptrs)
{
	unsigned char i;
	unsigned char tokens;

	tokens = count_tokens(rcvbuf);
	if (tokens > MAXTOKENS) tokens = MAXTOKENS;

	for (i=0; i<tokens; i++) {
		rcvbuf = scanfor_notspace(rcvbuf);
		if (!(*rcvbuf)) break;
		ptrs[i] = rcvbuf;
		rcvbuf = scanfor_space(rcvbuf);
		if (*rcvbuf) { *rcvbuf = 0; rcvbuf++; };
	}

	if (ptrs[0]) strupr((char*)ptrs[0]);
	return tokens;
}
