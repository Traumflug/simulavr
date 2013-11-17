#
#  $Id$
#

exampledir=$(docdir)/examples/simple_ex1

EXAMPLE=fred.elf
AVR_GCC=@AVR_GCC@
AVR_CPU=at90s8515
AVR_CFLAGS  = -g -O2 -mmcu=$(AVR_CPU)

example_DATA = $(EXAMPLE) fred.c README

CLEANFILES = $(EXAMPLE) fred.o

EXTRA_DIST = fred.c

SUFFIXES = .c

run_sim: $(EXAMPLE)
	../../src/simulavr --device $(AVR_CPU) -f $(EXAMPLE) \
	    -W 0x20,- -R 0x22,- -T exit

do:
if USE_TCL
	../simulavr.tcl -d $(AVR_CPU) -f $(EXAMPLE) \
	    -W 0x20,- -R 0x22,- -T exit
else
	@echo "Tcl not installed. Example will not run. Sorry!"
endif

dogdb:
if USE_TCL
	../simulavr.tcl -d $(AVR_CPU) -f $(EXAMPLE) \
	    -W 0x20,- -R 0x22,- -T exit -g
else
	@echo "Tcl not installed. Example will not run. Sorry!"
endif

.c.o:
	$(AVR_GCC) $(AVR_CFLAGS) -c -o $@ $<

fred.o: $(srcdir)/fred.c

$(EXAMPLE): fred.o
	$(AVR_GCC) $(AVR_CFLAGS) -o $@ $<

