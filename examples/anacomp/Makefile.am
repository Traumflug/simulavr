#
#  $Id$
#

examplesdir = $(docdir)/examples/anacomp

EXAMPLE=anacomp
AVR_GCC=@AVR_GCC@
PYTHON=@PYTHON@
AVR_CPU=at90s4433
AVR_CFLAGS  = -g -O2 -mmcu=$(AVR_CPU)

EXTRA_DIST = main.c checkdebug.py $(EXAMPLE).tcl

examples_DATA = $(EXAMPLE).elf README Makefile $(EXTRA_DIST)

CLEANFILES = $(EXAMPLE).elf trace anacomp.o $(EXAMPLE).lst

do: $(EXAMPLE).elf
	../simulavr.tcl -d $(AVR_CPU) -f $(EXAMPLE).elf -u \
	    -s $(srcdir)/$(EXAMPLE).tcl

dogdb: $(EXAMPLE).elf
	../simulavr.tcl -d $(AVR_CPU) -f $(EXAMPLE).elf -u \
	    -s $(srcdir)/$(EXAMPLE).tcl -g

do_python: $(EXAMPLE).elf
	$(PYTHON) $(srcdir)/checkdebug.py

$(EXAMPLE).elf: main.o
	$(AVR_GCC) $(AVR_CFLAGS) -o $@ $<

%.o: $(srcdir)/%.c
	$(AVR_GCC) $(AVR_CFLAGS) -c -o $@ $<

main.o: $(srcdir)/main.c
