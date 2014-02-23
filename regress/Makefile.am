#
# $Id$
#

MAINTAINERCLEANFILES = Makefile.in stamp-vti

EXTRA_DIST           = README regress.py.in

SUBDIRS              = modules test_opcodes avrtest timertest extinttest gtest modtest

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
