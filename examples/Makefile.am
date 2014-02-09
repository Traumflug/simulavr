#
#  $Id$
#
#

examplesdir = $(docdir)/examples

SUBDIRS = simple_ex1 simple_serial
if USE_TCL
SUBDIRS += anacomp atmega48 spi stdiodemo feedback atmega128_timer atmel_key
endif
if PYTHON_USE
SUBDIRS += python
endif
if USE_VERILOG
SUBDIRS += verilog
endif

EXTRA_DIST = kbd.xbm

examples_DATA = gui.tcl simulavr.tcl ChangeLog kbd.xbm

all-local: $(builddir)/kbd.xbm

$(builddir)/kbd.xbm: $(srcdir)/kbd.xbm
	-test -r $(builddir)/kbd.xbm || cp $(srcdir)/kbd.xbm $(builddir)/kbd.xbm

