#
#  $Id$
#

EXAMPLE = example.elf
AVR_GCC = @AVR_GCC@
AVR_CPU = atmega128
AVR_CFLAGS  = -g -O2 -mmcu=$(AVR_CPU)

noinst_DATA = $(EXAMPLE)

CLEANFILES = $(EXAMPLE)

EXTRA_DIST = example.c example.py README

do_example: $(EXAMPLE)
	PYTHONPATH=../../src @PYTHON@ example.py

$(EXAMPLE): example.c
	$(AVR_GCC) $(AVR_CFLAGS) -o $@ $<

