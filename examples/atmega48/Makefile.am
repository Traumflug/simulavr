#
#  $Id$
#

examplesdir = $(docdir)/examples/atmega48

EXAMPLE=atmega48
AVR_GCC=@AVR_GCC@
AVR_CPU=atmega48
AVR_CFLAGS  = -g -Os -mmcu=$(AVR_CPU)

EXTRA_DIST = main.cpp spidata anadata1 anadata2 anadata3 $(EXAMPLE).tcl

SUFFIXES = .cpp

examples_DATA = $(EXAMPLE).elf $(EXTRA_DIST) README

CLEANFILES = $(EXAMPLE).elf main.o trace

all-local: $(objdir)/spidata $(objdir)/anadata1 \
    $(objdir)/anadata2 $(objdir)/anadata3

$(objdir)/spidata: $(srcdir)/spidata
	test -r spidata || cp $(srcdir)/spidata spidata

$(objdir)/anadata1: $(srcdir)/anadata1
	test -r anadata1 || cp $(srcdir)/anadata1 anadata1

$(objdir)/anadata2: $(srcdir)/anadata2
	test -r anadata2 || cp $(srcdir)/anadata2 anadata2

$(objdir)/anadata3: $(srcdir)/anadata3
	test -r anadata3 || cp $(srcdir)/anadata3 anadata3

# 1000000000ns/1000000MHz == 1000ns
do: $(EXAMPLE).elf
	../simulavr.tcl -d $(AVR_CPU) -f $(EXAMPLE).elf -F 1000 \
	    -s $(srcdir)/$(EXAMPLE).tcl

dogdb: $(EXAMPLE).elf
	../simulavr.tcl -d $(AVR_CPU) -f $(EXAMPLE).elf -F 1000 \
	    -s $(srcdir)/$(EXAMPLE).tcl -g

# remove when do/dogdb work
do_old:
	$(TCL_WISH) check.tcl

.cpp.o:
	$(AVR_GCC) $(AVR_CFLAGS) -c -o $@ $<

main.o : $(srcdir)/main.cpp

$(EXAMPLE).elf : main.o
	$(AVR_GCC) $(AVR_CFLAGS) -o $@ $<
