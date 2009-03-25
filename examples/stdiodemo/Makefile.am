#
#  $Id$
#

AVR_GCC=@AVR_GCC@
AVR_CPU=atmega128
AVR_CPU_CFLAGS =-mmcu=$(AVR_CPU)
AVR_CFLAGS = $(AVR_CPU_CFLAGS) -Os -Wall -DF_CPU=3686400UL

noinst_DATA = stdiodemo.elf

CLEANFILES = stdiodemo.elf hd44780.o lcd.o stdiodemo.o uart.o

EXTRA_DIST = defines.h hd44780.c hd44780.h lcd.c lcd.h Makefile.am \
  README stdiodemo.c stdiodemo.dox stdiodemo-setup.jpg uart.c uart.h

# Frequency 271 ==> ~3.686400 MHz
do: stdiodemo.elf
	../simulavr.tcl -d $(AVR_CPU) -f stdiodemo.elf -u -F 271 \
	    -s $(srcdir)/stdiodemo.tcl

dogdb: stdiodemo.elf
	../simulavr.tcl -d $(AVR_CPU) -f stdiodemo.elf -u -F 271\
	    -s $(srcdir)/stdiodemo.tcl -g

stdiodemo.elf: hd44780.o lcd.o stdiodemo.o uart.o
	$(AVR_GCC) $(AVR_CPU_CFLAGS) -o $@ $^ -lm

%.o: $(srcdir)/%.c
	$(AVR_GCC) $(AVR_CFLAGS) -c -o $@ $<

hd44780.o: hd44780.c
lcd.o: lcd.c
stdiodemo.o: stdiodemo.c
uart.o: uart.c
