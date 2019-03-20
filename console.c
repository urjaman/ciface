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

#define CR 0x0D
#define LF 0x0A
#define BS 0x08
#define DEL 0x7F
#define SPACE 0x20


/* a few frser ops for us to know for exiting */
#define NOP 0x00
#define IFACE 0x01
#define SYNCNOP 0x10


uint8_t getline(unsigned char *buf, unsigned char len)
{
	unsigned char val,i;
	memset(buf,0,len);
	for(i=0; i<len; i++) {
#ifdef ciface_peek
		val = ciface_peek();
#else
		val = ciface_recv();
#endif
		if ((val==NOP)||(val==IFACE)||
		    (val==SYNCNOP)) return 1; // EXIT
#ifdef ciface_peek
		val = ciface_recv();
#endif
		if (((val == BS)||(val == DEL))&&(i)) { ciface_send(BS); ciface_send(SPACE); ciface_send(BS); i = i-2; continue; }; // Understand BS or DEL
		if (val == CR) { ciface_send(CR); ciface_send(LF); buf[i] = 0; break; }; // Understand LF
		if ((val < 32)||(val == DEL)) { i--; continue; };
		if (val == 255) { ciface_recv(); ciface_recv(); i--; continue; }; // Filter TELNET options
		buf[i] = val;
		ciface_send(val);
	}
	buf[len-1] = 0;
	return 0;
}


uint8_t getline_mc(unsigned char *buf, unsigned char len)
{
	unsigned char val,i;
	for(i=getline_i; i<len; i++) {
		if (ciface_isdata()) {
			val = ciface_recv();
			if (((val == BS)||(val == DEL))&&(i)) { ciface_send(BS); ciface_send(SPACE); ciface_send(BS); i = i-2; continue; }; // Understand BS or DEL
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
