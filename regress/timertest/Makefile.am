#
# $Id$
#

MAINTAINERCLEANFILES = Makefile.in stamp-vti

EXTRA_DIST = timertest.cfg timertest.template timer_16_extn.py timer_16_extp.py \
    timer_16_icap_n.py timer_16_icap_nc.py timer_16_icap_p.py timer_16_icap_pc.py \
    timer_16bit.c timer_16bit_ext.c timer_16bit_fastpwm.py timer_16bit_fastpwm.sig \
    timer_16bit_icap.c timer_16bit_normal.py timer_16bit_normal.sig \
    timer_16bit_normal_90.sig timer_8bit.c timer_8bit_ctc.c timer_8bit_ctc.py \
    timer_8bit_ctc.sig timer_8bit_normal.py timer_8bit_normal.sig \
    timer_8bit_normal_90.sig timer_tX5_8bit.c timer_tX5_8bit_ctc.py \
    timer_tX5_8bit_normal.py timer_tX5_8bit_pwm.py

export PYTHONPATH=$(srcdir)/../modules:$(srcdir)/../../src/python

check-local: timertest

clean-local:
	rm -f timertest.makefile
	rm -f $(srcdir)/*.py[co] $(srcdir)/*.elf $(srcdir)/*.vcd

timertest:
if PYTHON_USE
if USE_AVR_CROSS
	@PYTHON@ -m create_makefile -o $(srcdir)/timertest.makefile \
	                            -c $(srcdir)/timertest.cfg \
	                            -t $(srcdir)/timertest.template
	$(MAKE) -f timertest.makefile test
else
	@echo "  Configure could not find AVR cross compiling environment so timertest"
	@echo "  can not be run."
endif
else
	@echo "  Configure could not find python on your system so timertest"
	@echo "  can not be run."
endif

.PHONY: timertest

# EOF
