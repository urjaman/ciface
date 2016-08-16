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

#include "main.h"
#include "uart.h"
#include "ciface.h"
#include "console.h"
#include "appdb.h"

static const unsigned char helpstr[] PROGMEM = "?";
static void help_cmd(void);

#include "appdb_db.c"

static void invalid_command(void) {
	sendstr(tokenptrs[0]);
	sendstr_P(PSTR(": not found"));
}

static void help_cmd(void) {
	unsigned char i;
	const struct command_t * ctptr;
	PGM_P name;
	for(i=0;;i++) {
		ctptr = &(appdb[i]);
		name = (PGM_P)pgm_read_ptr(&(ctptr->name));
		if (!name) break;
		sendstr_P(name);
		SEND(' ');
	}
}

void *find_appdb(unsigned char* cmd) {
	unsigned char i;
	const struct command_t * ctptr;
	PGM_P name;
	void* fp;
	for(i=0;;i++) {
		ctptr = &(appdb[i]);
		name = (PGM_P)pgm_read_ptr(&(ctptr->name));
		fp = (void*)pgm_read_ptr(&(ctptr->function));
		if (!name) break;
		if (strcmp_P((char*)cmd,name) == 0) {
			return fp;
			}
	}
	return &(invalid_command);
}
