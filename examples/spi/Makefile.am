#
#  $Id$
#

EXAMPLE=spi
AVR_GCC=@AVR_GCC@
AVR_CPU=atmega128
AVR_CFLAGS  = -g -Os -mmcu=$(AVR_CPU)

noinst_DATA = $(EXAMPLE).elf

CLEANFILES = $(EXAMPLE).elf main.o trace

EXTRA_DIST = main.cpp spidata anadata $(EXAMPLE).tcl

all-local: $(objdir)/spidata $(objdir)/anadata

$(objdir)/spidata: $(srcdir)/spidata
	test -r spidata || cp $(srcdir)/spidata spidata

$(objdir)/anadata: $(srcdir)/anadata
	test -r anadata || cp $(srcdir)/anadata anadata

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

# WAR: should detect avr-toolset first!

%.o: $(srcdir)/%.cpp
	$(AVR_GCC) $(AVR_CFLAGS) -c -o $@ $<

main.o : $(srcdir)/main.cpp

$(EXAMPLE).elf : main.o
	$(AVR_GCC) $(AVR_CFLAGS) -o $@ $<
