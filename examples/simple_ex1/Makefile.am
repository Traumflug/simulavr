MAINTAINERCLEANFILES 	= Makefile.in

EXTRA_DIST           = fred.c

# WAR: should detect avr-toolset first!

fred.o : $(srcdir)/fred.c
	$(AVR_GCC) -Wa,-adhl=$@.lst -g -O2 -mmcu=at90s8515 -c -o $@ $< 

fred : fred.o
	$(AVR_GCC) -Wl,-M,-Map=$@.map -mmcu=at90s8515 -o $@ $<

run_sim: fred
	../../src/simulavr --device at90s8515 -f fred -W 0x20,- -R 0x22,- -T exit


SUBDIRS =
