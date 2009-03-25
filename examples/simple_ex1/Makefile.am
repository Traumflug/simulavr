#
#  $Id$
#

AVR_GCC=@AVR_GCC@

noinst_DATA = fred

CLEANFILES = fred fred.o trace

EXTRA_DIST = fred.c

## If you want a listing and map, uncomment these
## LIST_ARG=-Wa,-adhl=$@.lst
## MAP_ARG=-Wl,-M,-Map=$@.map

AVR_MCU=at90s8515

fred.o : $(srcdir)/fred.c
	$(AVR_GCC) $(LIST_ARG) -g -O2 -mmcu=$(AVR_MCU) -c -o $@ $< 

fred : fred.o
	$(AVR_GCC) $(MAP_ARG) -mmcu=$(AVR_MCU) -o $@ $<

run_sim: fred
	../../src/simulavr --device $(AVR_MCU) -f fred \
	    -W 0x20,- -R 0x22,- -T exit

do:
	../simulavr.tcl -d $(AVR_MCU) -f fred \
	    -W 0x20,- -R 0x22,- -T exit

dogdb:
	../simulavr.tcl -d $(AVR_MCU) -f fred \
	    -W 0x20,- -R 0x22,- -T exit -g
