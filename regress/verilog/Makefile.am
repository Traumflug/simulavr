#
# $Id$
#

verilogdir = $(srcdir)/regress/verilog

EXTRA_DIST = baretest.v toggle.c verilog-test.py

export PYTHONPATH=$(srcdir)/../modules

if USE_AVR_CROSS

toggle_PROG = toggle.elf

verilog_DATA = $(EXTRA_DIST) $(toggle_PROG) Makefile

verilog: $(toggle_PROG)

.c.o:
	$(AVR_GCC) -mmcu=attiny2313 -c -Os $^ -o $@

.o.elf:
	$(AVR_GCC) -mmcu=attiny2313 $^ -o $@

verilogtest: $(toggle_PROG)
if USE_VERILOG_TOOLS
	$(IVERILOG) baretest.v -s test -v -o baretest.vvp
	$(VVP) -M../../src -mavr baretest.vvp
	@PYTHON@ verilog-test.py
else
	@echo "  Configure could not find verilog tools to run this test"
endif

check-local: verilogtest

endif

clean-local:
	rm -f toggle.elf baretest.vvp baretest.vcd

.PHONY: verilogtest

# EOF
