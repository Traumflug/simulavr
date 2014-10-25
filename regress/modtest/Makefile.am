#
# $Id$
#

MAINTAINERCLEANFILES = Makefile.in

EXTRA_DIST = modtest.cfg modtest.template pin.py anacomp.c anacomp.py adc.c adc.py adc_int.c adc_int.py \
             adc_fr.c adc_fr.py adc_diff.c adc_diff.py anacomp_int.c anacomp_int.py anacomp_mux.c \
             anacomp_mux.py adc_gain.py adc_diff_t25.c adc_diff_t25.py port.c port.py eeprom.c eeprom.py \
             eeprom_int.c eeprom_int.py

export PYTHONPATH=$(srcdir)/../modules:$(srcdir)/../../src/python

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
