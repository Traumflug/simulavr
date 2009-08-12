#
# $Id$
#

MAINTAINERCLEANFILES = Makefile.in stamp-vti

EXTRA_DIST = timertest.cfg timertest.template

export PYTHONPATH=$(srcdir)/../modules

check-local: timertest

clean-local:
	rm -f timertest.makefile
	rm -f $(srcdir)/*.py[co] $(srcdir)/*.elf $(srcdir)/*.vcd

timertest:
if PYTHON_USE
	@PYTHON@ -m create_makefile -o $(srcdir)/timertest.makefile \
	                            -c $(srcdir)/timertest.cfg \
	                            -t $(srcdir)/timertest.template
	$(MAKE) -f timertest.makefile test
else
	@echo "  Configure could not find python on your system so timertest"
	@echo "  can not be run."
endif

.PHONY: timertest

# EOF
