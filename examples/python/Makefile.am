#
#  $Id$
#

AVR_GCC = @AVR_GCC@

if USE_AVR_CROSS
if PYTHON_USE

EXAMPLE = example.elf
AVR_CPU = atmega128
AVR_CFLAGS  = -g -O2 -mmcu=$(AVR_CPU)

run_example: $(EXAMPLE)
	PYTHONPATH=../../src @PYTHON@ example.py

$(EXAMPLE): example.c
	$(AVR_GCC) $(AVR_CFLAGS) -o $@ $<

else

run_example:
	@echo "example not available, because python isn't configured"

endif

else

run_example:
	@echo "example not available, because avr cross compilation not available"

endif


noinst_DATA = $(EXAMPLE)

CLEANFILES = $(EXAMPLE)

EXTRA_DIST = example.c example.py README

example: run_example

.PHONY: example

