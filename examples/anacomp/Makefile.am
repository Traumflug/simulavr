#
#  $Id$
#

examplesdir = $(docdir)/examples/anacomp

EXAMPLE=anacomp
AVR_GCC=@AVR_GCC@
PYTHON=@PYTHON@
AVR_CPU=at90s4433
AVR_CFLAGS  = -g -O2 -mmcu=$(AVR_CPU)

EXTRA_DIST = main.c $(EXAMPLE).tcl

SUFFIXES = .c

examples_DATA = $(EXAMPLE).elf README Makefile $(EXTRA_DIST)

CLEANFILES = $(EXAMPLE).elf trace anacomp.o $(EXAMPLE).lst

do: $(EXAMPLE).elf
if TCL_ITCL_AVAILABLE
	../simulavr.tcl -d $(AVR_CPU) -f $(EXAMPLE).elf -u \
	    -s $(srcdir)/$(EXAMPLE).tcl
else
	@echo "Itcl package not installed. Example will not run. Sorry!"
endif

dogdb: $(EXAMPLE).elf
if TCL_ITCL_AVAILABLE
	../simulavr.tcl -d $(AVR_CPU) -f $(EXAMPLE).elf -u \
	    -s $(srcdir)/$(EXAMPLE).tcl -g
else
	@echo "Itcl package not installed. Example will not run. Sorry!"
endif

$(EXAMPLE).elf: main.o
	$(AVR_GCC) $(AVR_CFLAGS) -o $@ $<

.c.o:
	$(AVR_GCC) $(AVR_CFLAGS) -c -o $@ $<

main.o: $(srcdir)/main.c
