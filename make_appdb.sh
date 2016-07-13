#!/bin/sh
grep CIFACE_APP $@ > appdb_list.tmp
echo '#define CIFACE_APP(fn,an) static const unsigned char fn##_str[] PROGMEM = an;'
cat appdb_list.tmp
echo '#undef CIFACE_APP'
echo '#define CIFACE_APP(fn,an) void fn(void);'
cat appdb_list.tmp
echo '#undef CIFACE_APP'
echo '#define CIFACE_APP(fn,an) {(PGM_P)fn##_str, &(fn)},'
echo 'const struct command_t appdb[] PROGMEM = {'
cat appdb_list.tmp
echo '{(PGM_P)helpstr, &(help_cmd)}, {NULL,NULL}};'
echo '#undef CIFACE_APP'
rm appdb_list.tmp