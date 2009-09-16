#
#  $Id$
#

AVR_GCC = @AVR_GCC@

if USE_AVR_CROSS
if PYTHON_USE

AVR_CPU = atmega128
AVR_CFLAGS  = -g -O2 -mmcu=$(AVR_CPU)

# example1: unit test
EXAMPLE1 = example.elf

$(EXAMPLE1): example.c
	$(AVR_GCC) $(AVR_CFLAGS) -o $@ $<

example1: $(EXAMPLE1)
	PYTHONPATH=../../src @PYTHON@ example.py $(AVR_CPU):$<

# example2: usage of Pin and Net
example2:
	PYTHONPATH=../../src @PYTHON@ example_pin.py

# example3: show pin output and inject change of port pin
EXAMPLE3 = example_io.elf

$(EXAMPLE3): example_io.c
	$(AVR_GCC) $(AVR_CFLAGS) -o $@ $<

example3: $(EXAMPLE3)
	PYTHONPATH=../../src @PYTHON@ example_io.py $(AVR_CPU):$<

# example4: closed to example3, but don't inject signals to port
EXAMPLE4 = ex_pinout.elf

$(EXAMPLE4): ex_pinout.c
	$(AVR_GCC) $(AVR_CFLAGS) -o $@ $<

example4: $(EXAMPLE4)
	PYTHONPATH=../../src @PYTHON@ ex_pinout.py $(AVR_CPU):$<

run_example: example1 example2 example3 example4

else

run_example:
	@echo "example not available, because python isn't configured"

endif

else

run_example:
	@echo "example not available, because avr cross compilation not available"

endif


noinst_DATA = $(EXAMPLE1) $(EXAMPLE3) $(EXAMPLE4) ex_utils.pyc ex_pinout.vcd

CLEANFILES = $(EXAMPLE1) $(EXAMPLE3) $(EXAMPLE4) ex_utils.pyc ex_pinout.vcd

EXTRA_DIST = example.c example.py example_pin.py example_io.c example_io.py  ex_utils.py ex_pinout.py ex_pinout.c README

example: run_example

.PHONY: example

