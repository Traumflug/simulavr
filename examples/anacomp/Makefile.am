#
#  $Id$
#

AVR_GCC=@AVR_GCC@
PYTHON=@PYTHON@

noinst_DATA = anacomp

CLEANFILES = anacomp anacomp.o trace

EXTRA_DIST = main.c checkdebug.py

AVR_CPU=at90s4433

do: anacomp
	../simulavr.tcl -d $(AVR_CPU) -f anacomp -u \
	    -s $(srcdir)/anacomp.tcl

dogdb: anacomp
	../simulavr.tcl -d $(AVR_CPU) -f anacomp -u \
	    -s $(srcdir)/anacomp.tcl -g

do_python: anacomp
	$(PYTHON) $(srcdir)/checkdebug.py

anacomp.o : $(srcdir)/main.c
	$(AVR_GCC) -g -O2 -mmcu=$(AVR_CPU) -c -o $@ $<

anacomp : anacomp.o
	$(AVR_GCC) -mmcu=$(AVR_CPU) -o $@ $<

SUBDIRS =
