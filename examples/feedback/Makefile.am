#
#  $Id$
#

examplesdir = $(docdir)/examples/feedback

EXAMPLE=feedback

AVR_GCC=@AVR_GCC@
AVR_CPU=atmega128
AVR_FREQUENCY=4000000
AVR_CFLAGS= -g -O2 -mmcu=$(AVR_CPU) -DF_CPU=$(AVR_FREQUENCY)
AVR_OBJS=main.o debugio.o uart.o adc.o
SIMULAVR_ARGS=-d $(AVR_CPU) -f $(EXAMPLE).elf -s $(srcdir)/$(EXAMPLE).tcl \
    -W 0x20,/dev/stderr -R 0x22,- -F $(AVR_FREQUENCY) -T exit \
    -S ./simfeedback.tcl

EXTRA_DIST = main.c $(EXAMPLE).tcl debugio.c debugio.h adc.c uart.c uart.h \
    defines.h simfeedback.tcl

SUFFIXES = .c

examples_DATA = $(EXAMPLE).elf $(EXTRA_DIST) README

CLEANFILES = $(EXAMPLE).elf $(AVR_OBJS) trace $(EXAMPLE).lst

do: $(EXAMPLE).elf
	../simulavr.tcl $(SIMULAVR_ARGS)

dogdb: $(EXAMPLE).elf
	../simulavr.tcl $(SIMULAVR_ARGS) -g

$(EXAMPLE).elf: $(AVR_OBJS)
	$(AVR_GCC) $(AVR_CFLAGS) -o $@ $^

.c.o:
	$(AVR_GCC) $(AVR_CFLAGS) -c -o $@ $<

main.o: main.c
debugio.o: debugio.c
uart.o: uart.c
