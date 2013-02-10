#
# $Id$
#

MAINTAINERCLEANFILES = Makefile.in

EXTRA_DIST = modtest.cfg modtest.template pin.py anacomp.c anacomp.py

export PYTHONPATH=$(srcdir)/../modules:$(srcdir)/../../src

check-local: modtest

clean-local:
	rm -f modtest.makefile
	rm -f $(srcdir)/*.py[co] $(srcdir)/*.elf

modtest:
if PYTHON_USE
if USE_AVR_CROSS
	@PYTHON@ -m create_makefile -o $(srcdir)/modtest.makefile \
	                            -c $(srcdir)/modtest.cfg \
	                            -t $(srcdir)/modtest.template
	$(MAKE) -f modtest.makefile test
else
	@echo "  Configure could not find AVR cross compiling environment so timertest"
	@echo "  can not be run."
endif
else
	@echo "  Configure could not find python on your system so timertest"
	@echo "  can not be run."
endif

.PHONY: modtest

# EOF
