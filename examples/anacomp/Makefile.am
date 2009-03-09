#
#  $Id$
#

AVR_GCC=@AVR_GCC@
PYTHON=@PYTHON@

noinst_DATA = anacomp

CLEANFILES = anacomp anacomp.o trace

do: anacomp
	$(TCL_WISH) check.tcl

dogdb: anacomp
	$(TCL_WISH) checkdebug.tcl

do_python: anacomp
	$(PYTHON) $(srcdir)/checkdebug.py


# WAR: should detect avr-toolset first!

anacomp.o : $(srcdir)/main.c
	$(AVR_GCC) -g -O2 -mmcu=at90s4433 -c -o $@ $<

anacomp : anacomp.o
	$(AVR_GCC) -mmcu=at90s4433 -o $@ $<

SUBDIRS =
