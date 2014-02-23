#
#  $Id$
#

EXTRA_DIST = avrtest.cfg avrtest.template run_test.sh avrtest_help.h \
    avrtest_help.c test_abort.c test_exit.c test_maxruntime.c

export PYTHONPATH=$(srcdir)/../modules

check-local: avrtest

clean-local:
	rm -f avrtest.makefile
	rm -f $(srcdir)/*.elf $(srcdir)/*.report $(srcdir)/*.output

avrtest:
if PYTHON_CMD_USE
if USE_AVR_CROSS
	@PYTHON@ -m create_makefile -o $(srcdir)/avrtest.makefile \
	                            -c $(srcdir)/avrtest.cfg \
	                            -t $(srcdir)/avrtest.template
	$(MAKE) -f avrtest.makefile GNUTIME=@GNUTIME@ test
else
	@echo "  Configure could not find AVR cross compiling environment so avrtest"
	@echo "  can not be run."
endif
else
	@echo "  Configure could not find python on your system so avrtest"
	@echo "  can not be run."
endif

.PHONY: avrtest


