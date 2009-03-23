#
#  $Id$
#

AVR_GCC=@AVR_GCC@

noinst_DATA = spi.elf

CLEANFILES = spi.elf main.o trace

EXTRA_DIST = main.cpp spidata anadata

all-local: $(objdir)/spidata $(objdir)/anadata

$(objdir)/spidata: $(srcdir)/spidata
	test -r spidata || cp $(srcdir)/spidata spidata

$(objdir)/anadata: $(srcdir)/anadata
	test -r anadata || cp $(srcdir)/anadata anadata

do: spi.elf
	$(TCL_WISH) check.tcl

# WAR: should detect avr-toolset first!

main.o : $(srcdir)/main.cpp
	$(AVR_GCC) -g -O2 -mmcu=atmega128 -c -o $@ $<

spi.elf : main.o
	$(AVR_GCC) -mmcu=atmega128 -o $@ $<

SUBDIRS =
