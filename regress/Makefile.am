#
# $Id$
#

MAINTAINERCLEANFILES = Makefile.in stamp-vti

EXTRA_DIST           = README regress.py.in

SUBDIRS              = modules test_opcodes

if USE_AVR_CROSS

SUBDIRS += avrtest gtest

if PYTHON_USE
SUBDIRS += timertest extinttest modtest
endif

if USE_VERILOG
if PYTHON_CMD_USE
SUBDIRS += verilog
endif
endif

if USE_TCL
SUBDIRS += tcl
endif

endif

all:

check-local: regression

clean-local:
	rm -f *.py[co] *.err *.out
	rm -f $(srcdir)/modules/*.py[co]
	rm -f $(srcdir)/test_*/*.py[co]

regression:
if PYTHON_CMD_USE
	@PYTHON@ ./regress.py 2> regress.err | tee regress.out
else
	@echo "  Configure could not find python on your system so regression"
	@echo "  tests can not be automated."
endif
