#
#  $Id$
#

examplesdir = $(docdir)/examples/python

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
	PYTHONPATH=../../src/python @PYTHON@ example.py $(AVR_CPU):$<

# example2: usage of Pin and Net
example2:
	PYTHONPATH=../../src/python @PYTHON@ example_pin.py

# example3: show pin output and inject change of port pin
EXAMPLE3 = example_io.elf

$(EXAMPLE3): example_io.c
	$(AVR_GCC) $(AVR_CFLAGS) -o $@ $<

example3: $(EXAMPLE3)
	PYTHONPATH=../../src/python @PYTHON@ example_io.py $(AVR_CPU):$<

# example4: closed to example3, but don't inject signals to port
EXAMPLE4 = ex_pinout.elf

$(EXAMPLE4): ex_pinout.c
	$(AVR_GCC) $(AVR_CFLAGS) -o $@ $<

example4: $(EXAMPLE4)
	PYTHONPATH=../../src/python @PYTHON@ ex_pinout.py $(AVR_CPU):$<

# example5: multicore example with 2 ATmega16
EXAMPLE5A = multicore_a.elf
EXAMPLE5B = multicore_b.elf
EX5_CFLAGS = -g -O2 -mmcu=atmega16

$(EXAMPLE5A): multicore.c
	$(AVR_GCC) $(EX5_CFLAGS) -DDUAL_A=1 -o $@ $<

$(EXAMPLE5B): multicore.c
	$(AVR_GCC) $(EX5_CFLAGS) -DDUAL_B=1 -o $@ $<

example5: $(EXAMPLE5A) $(EXAMPLE5B)
	PYTHONPATH=../../src/python @PYTHON@ multicore.py

# example6: ADC with atmega16
EXAMPLE6 = adc.elf
EX6_CFLAGS = -g -O2 -mmcu=atmega16

$(EXAMPLE6): adc.c
	$(AVR_GCC) $(EX6_CFLAGS) -o $@ $<

example6: $(EXAMPLE6)
	PYTHONPATH=../../src/python @PYTHON@ adc.py atmega16:$<

run_example: example1 example2 example3 example4 example5 example6

else

run_example:
	@echo "example not available, because python isn't configured"

endif

else

run_example:
	@echo "example not available, because avr cross compilation not available"

endif


EXTRA_DIST = example.c example.py example_pin.py example_io.c adc.c \
             example_io.py ex_utils.py ex_pinout.py ex_pinout.c multicore.c \
             multicore.py adc.py

examples_DATA = $(EXAMPLE1) $(EXAMPLE3) $(EXAMPLE4) $(EXAMPLE5A) $(EXAMPLE5B) \
                $(EXAMPLE6) $(EXTRA_DIST) README Makefile

CLEANFILES = $(EXAMPLE1) $(EXAMPLE3) $(EXAMPLE4) $(EXAMPLE5A) $(EXAMPLE5B) \
             $(EXAMPLE6) ex_utils.pyc ex_pinout.vcd

example: run_example

.PHONY: example

