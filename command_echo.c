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
#include "lib.h"
#include "ciface.h"
#include "appdb.h"

CIFACE_APP(echo_cmd, "ECHO")
{
	unsigned char i;
	for (i=1; i<token_count; i++) {
		sendstr(tokenptrs[i]);
		ciface_send(' ');
	}
}
