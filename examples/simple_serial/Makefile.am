#
#  $Id$
#

examplesdir = $(docdir)/examples/simple_serial

EXAMPLE = simple_serial

AVR_GCC = @AVR_GCC@
AVR_CPU = atmega644
AVR_FREQUENCY = 20000000
AVR_CFLAGS = -g -O2 -mmcu=$(AVR_CPU) -DF_CPU=$(AVR_FREQUENCY)
AVR_OBJS = main.o serial.o
SIMULAVR_ARGS = -d $(AVR_CPU) -f $(EXAMPLE).elf

EXTRA_DIST = main.c serial.c serial.h

examples_DATA = $(EXAMPLE).hex $(EXAMPLE).elf $(EXTRA_DIST) README

CLEANFILES = $(EXAMPLE).hex $(EXAMPLE).elf $(AVR_OBJS)


do: $(EXAMPLE).elf
	../../src/simulavr $(SIMULAVR_ARGS)

program: $(EXAMPLE).hex
	avrdude -c stk500v2 -b 115200 -p $(AVR_CPU) -P $(SERIAL_AVR) \
	-U flash:w:$<


$(EXAMPLE).hex: $(EXAMPLE).elf
	avr-objcopy -j .text -j .data -O ihex -R .eeprom -R .fuse -R .lock $< $@

$(EXAMPLE).elf: $(AVR_OBJS)
	$(AVR_GCC) $(AVR_CFLAGS) -o $@ $^

main.o: $(srcdir)/main.c
	$(AVR_GCC) $(AVR_CFLAGS) -c -o $@ $<

serial.o: $(srcdir)/serial.c  $(srcdir)/serial.h
	$(AVR_GCC) $(AVR_CFLAGS) -c -o $@ $<

