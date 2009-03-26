#
#  $Id$
#

AVR_GCC=@AVR_GCC@
AVR_CPU=atmega128

noinst_DATA = spi.elf

CLEANFILES = spi.elf main.o trace

EXTRA_DIST = main.cpp spidata anadata spi.tcl

all-local: $(objdir)/spidata $(objdir)/anadata

$(objdir)/spidata: $(srcdir)/spidata
	test -r spidata || cp $(srcdir)/spidata spidata

$(objdir)/anadata: $(srcdir)/anadata
	test -r anadata || cp $(srcdir)/anadata anadata

# 1000000000ns/1000000MHz == 1000ns
do: spi.elf
	../simulavr.tcl -d $(AVR_CPU) -f spi.elf -u -F 1000 \
	    -s $(srcdir)/spi.tcl

dogdb: spi.elf
	../simulavr.tcl -d $(AVR_CPU) -f spi.elf -u -F 1000 \
	    -s $(srcdir)/spi.tcl -g

# remove when do/dogdb work
do_old:
	$(TCL_WISH) check.tcl

# WAR: should detect avr-toolset first!

main.o : $(srcdir)/main.cpp
	$(AVR_GCC) -g -O2 -mmcu=$(AVR_CPU) -c -o $@ $<

spi.elf : main.o
	$(AVR_GCC) -mmcu=$(AVR_CPU) -o $@ $<

SUBDIRS =
