#
#  $Id$
#

AVR_GCC=@AVR_GCC@

noinst_DATA = fred

CLEANFILES = fred fred.o trace

## If you want a listing and map, uncomment these
## LIST_ARG=-Wa,-adhl=$@.lst
## MAP_ARG=-Wl,-M,-Map=$@.map

fred.o : $(srcdir)/fred.c
	$(AVR_GCC) $(LIST_ARG) -g -O2 -mmcu=at90s8515 -c -o $@ $< 

fred : fred.o
	$(AVR_GCC) $(MAP_ARG) -mmcu=at90s8515 -o $@ $<

run_sim: fred
	../../src/simulavr --device at90s8515 -f fred \
	    -W 0x20,- -R 0x22,- -T exit
