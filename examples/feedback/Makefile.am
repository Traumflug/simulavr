#
#  $Id$
#

EXAMPLE=feedback
AVR_GCC=@AVR_GCC@
PYTHON=@PYTHON@
AVR_CPU=atmega128
AVR_FREQUENCY=4000000
AVR_CFLAGS= -g -O2 -mmcu=$(AVR_CPU) -DF_CPU=$(AVR_FREQUENCY)
AVR_OBJS=main.o debugio.o uart.o
SIMULAVR_ARGS=-d $(AVR_CPU) -f $(EXAMPLE) -s $(srcdir)/$(EXAMPLE).tcl \
    -W 0x20,/dev/stderr -R 0x22,- -F $(AVR_FREQUENCY) -T exit \
    -S ./simfeedback.tcl

noinst_DATA = $(EXAMPLE)

CLEANFILES = $(EXAMPLE) $(AVR_OBJS) trace $(EXAMPLE).lst

EXTRA_DIST = main.c $(EXAMPLE).tcl

do: $(EXAMPLE)
	../simulavr.tcl $(SIMULAVR_ARGS)

dogdb: $(EXAMPLE)
	../simulavr.tcl $(SIMULAVR_ARGS) -g

$(EXAMPLE): $(AVR_OBJS)
	$(AVR_GCC) $(AVR_CFLAGS) -o $@ $^

%.o: %.c
	$(AVR_GCC) $(AVR_CFLAGS) -c -o $@ $<

main.o: main.c
debugio.o: debugio.c
uart.o: uart.c
