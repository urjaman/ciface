#pragma once

struct command_t {
	PGM_P name;
	void(*function)(void);
};
extern const struct command_t appdb[] PROGMEM;

void *find_appdb(unsigned char* cmd);

/* This does more than is apparent; used by make_appdb.sh to generate the list. */
#define CIFACE_APP(fn,an) void fn(void); void fn(void)
