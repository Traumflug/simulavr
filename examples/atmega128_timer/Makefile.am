#
#  $Id$
#

examplesdir = $(docdir)/examples/atmega128_timer

EXAMPLE=timer.elf
AVR_GCC=@AVR_GCC@
AVR_CPU=atmega128
AVR_FREQUENCY=4000000
AVR_CFLAGS= -g -Os -mmcu=$(AVR_CPU) -DF_CPU=$(AVR_FREQUENCY)

EXTRA_DIST = debugio.c debugio.h main.c

SUFFIXES = .c

examples_DATA = $(EXAMPLE) $(EXTRA_DIST) README

CLEANFILES = $(EXAMPLE) main.o debugio.o trace

do:
	../simulavr.tcl -d $(AVR_CPU) -f $(EXAMPLE) \
	    -W 0x20,- -R 0x22,- -T exit

dogdb:
	../simulavr.tcl -d $(AVR_CPU) -f $(EXAMPLE) \
	    -W 0x20,- -R 0x22,- -T exit -g

.c.o:
	$(AVR_GCC) $(AVR_CFLAGS) -c -o $@ $<

main.o: $(srcdir)/main.c
debugio.o: $(srcdir)/debugio.c

$(EXAMPLE): main.o debugio.o
	$(AVR_GCC) $(AVR_CFLAGS) -o $@ $^

