#
#  $Id$
#

AVR_GCC=@AVR_GCC@
AVR_CPU_CFLAGS =-mmcu=atmega128
AVR_CFLAGS = $(AVR_CPU_CFLAGS) -Os -Wall -DF_CPU=3686400UL

noinst_DATA = stdiodemo.elf

CLEANFILES = stdiodemo.elf hd44780.o lcd.o stdiodemo.o uart.o

EXTRA_DIST = checkdebug.gdb checkdebug.tcl.in defines.h hd44780.c hd44780.h \
  lcd.c lcd.h Makefile.am Makefile.notauto README stdiodemo.c stdiodemo.dox \
  stdiodemo-setup.jpg uart.c uart.h

do: stdiodemo.elf
	$(TCL_WISH) check.tcl

dogdb: stdiodemo.elf
	$(TCL_WISH) checkdebug.tcl

stdiodemo.elf: hd44780.o lcd.o stdiodemo.o uart.o
	$(AVR_GCC) $(AVR_CPU_CFLAGS) -o $@ $^ -lm

%.o: $(srcdir)/%.c
	$(AVR_GCC) $(AVR_CFLAGS) -c -o $@ $<

hd44780.o: hd44780.c
lcd.o: lcd.c
stdiodemo.o: stdiodemo.c
uart.o: uart.c
