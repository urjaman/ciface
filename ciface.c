/*
 * Copyright (C) 2013 Urja Rannikko <urjaman@gmail.com>
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

#include "main.h"
#include "uart.h"
#include "console.h"
#include "appdb.h"
#include "lib.h"
#include "ciface.h"


const unsigned char prompt[] PROGMEM = "\x0D\x0A> ";
struct ciface_info ciface_mi;
#ifdef MULTI_CIFACE
struct ciface_info *ciface_ip;
#endif

void ciface_main(void)
{
	for (;;) {
		sendstr_P((PGM_P)prompt);
		if (getline(ciface_recvbuf,RECVBUFLEN)) return;
		token_count = tokenize(ciface_recvbuf,tokenptrs);
		if (token_count) {
			void(*func)(void);
			func = find_appdb(tokenptrs[0]);
			func();
		}
	}
}

void ciface_init(void) {
	sendstr_P((PGM_P)prompt);
	getline_i = 0;
}

void ciface_run(void) {
	if (getline_mc(ciface_recvbuf, RECVBUFLEN)) {
		token_count = tokenize(ciface_recvbuf,tokenptrs);
		if (token_count) {
			void(*func)(void);
			func = find_appdb(tokenptrs[0]);
			func();
		}
		sendstr_P((PGM_P)prompt);
	}
}


#ifndef MULTI_CIFACE
void ciface_yield(void) { }
#endif
