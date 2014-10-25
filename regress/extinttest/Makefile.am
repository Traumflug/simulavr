#
# $Id$
#

MAINTAINERCLEANFILES = Makefile.in stamp-vti

EXTRA_DIST = ext.cfg ext.template ext_int0.c ext_pcint.c ext_int0.py \
    ext_int1.py ext_int2.py ext_pcint.py

export PYTHONPATH=$(srcdir)/../modules:$(srcdir)/../../src/python

check-local: extinttest

clean-local:
	rm -f ext.makefile
	rm -f $(srcdir)/*.py[co] $(srcdir)/*.elf $(srcdir)/*.vcd

extinttest:
if PYTHON_USE
if USE_AVR_CROSS
	@PYTHON@ -m create_makefile -o $(srcdir)/ext.makefile \
	                            -c $(srcdir)/ext.cfg \
	                            -t $(srcdir)/ext.template
	$(MAKE) -f ext.makefile test
else
	@echo "  Configure could not find AVR cross compiling environment so extinttest"
	@echo "  can not be run."
endif
else
	@echo "  Configure could not find python on your system so extinttest"
	@echo "  can not be run."
endif

.PHONY: extinttest

# EOF
