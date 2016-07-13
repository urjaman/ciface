struct command_t {
	PGM_P name;
	void(*function)(void);
};
extern const struct command_t appdb[] PROGMEM;

void *find_appdb(unsigned char* cmd);

#define CIFACE_APP(fn,an) void fn(void); void fn(void)
