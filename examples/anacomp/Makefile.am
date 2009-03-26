#
#  $Id$
#

EXAMPLE=anacomp
AVR_GCC=@AVR_GCC@
PYTHON=@PYTHON@
AVR_CPU=at90s4433
AVR_CFLAGS  = -g -O2 -mmcu=$(AVR_CPU)

noinst_DATA = $(EXAMPLE)

CLEANFILES = $(EXAMPLE) anacomp.o trace $(EXAMPLE).lst

EXTRA_DIST = main.c checkdebug.py $(EXAMPLE).tcl

do: $(EXAMPLE)
	../simulavr.tcl -d $(AVR_CPU) -f $(EXAMPLE) -u \
	    -s $(srcdir)/$(EXAMPLE).tcl

dogdb: $(EXAMPLE)
	../simulavr.tcl -d $(AVR_CPU) -f $(EXAMPLE) -u \
	    -s $(srcdir)/$(EXAMPLE).tcl -g

do_python: $(EXAMPLE)
	$(PYTHON) $(srcdir)/checkdebug.py

$(EXAMPLE): main.o
	$(AVR_GCC) $(AVR_CFLAGS) -o $@ $<

%.o: $(srcdir)/%.c
	$(AVR_GCC) $(AVR_CFLAGS) -c -o $@ $<

main.o: $(srcdir)/main.c
