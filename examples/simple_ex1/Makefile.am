#
#  $Id$
#

EXAMPLE=fred
AVR_GCC=@AVR_GCC@
AVR_CPU=at90s8515
AVR_CFLAGS  = -g -O2 -mmcu=$(AVR_CPU)

noinst_DATA = $(EXAMPLE)

CLEANFILES = $(EXAMPLE) fred.o trace

EXTRA_DIST = fred.c

run_sim: $(EXAMPLE)
	../../src/simulavr --device $(AVR_MCU) -f $(EXAMPLE) \
	    -W 0x20,- -R 0x22,- -T exit

do:
	../simulavr.tcl -d $(AVR_MCU) -f $(EXAMPLE) \
	    -W 0x20,- -R 0x22,- -T exit

dogdb:
	../simulavr.tcl -d $(AVR_MCU) -f $(EXAMPLE) \
	    -W 0x20,- -R 0x22,- -T exit -g

%.o: $(srcdir)/%.c
	$(AVR_GCC) $(AVR_CFLAGS) -c -o $@ $<

fred.o: $(srcdir)/fred.c

$(EXAMPLE): fred.o
	$(AVR_GCC) $(AVR_CFLAGS) -o $@ $<

