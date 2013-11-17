#
#  $Id$
#

examplesdir = $(docdir)/examples/stdiodemo

AVR_GCC=@AVR_GCC@
EXAMPLE=stdiodemo
AVR_CPU=atmega128
AVR_CPU_CFLAGS =-mmcu=$(AVR_CPU)
AVR_CFLAGS = $(AVR_CPU_CFLAGS) -Os -Wall -DF_CPU=3686400UL

EXTRA_DIST = defines.h hd44780.c hd44780.h lcd.c lcd.h stdiodemo.tcl \
  README stdiodemo.c stdiodemo.dox stdiodemo-setup.jpg uart.c uart.h \
  checkdebug.gdb

SUFFIXES = .c

examples_DATA = $(EXAMPLE).elf $(EXTRA_DIST)

CLEANFILES = $(EXAMPLE).elf hd44780.o lcd.o stdiodemo.o uart.o

# Frequency 271 ==> ~3.686400 MHz
do: $(EXAMPLE).elf
if TCL_ITCL_AVAILABLE
	../simulavr.tcl -d $(AVR_CPU) -f $(EXAMPLE).elf -u -F 271 \
	    -s $(srcdir)/$(EXAMPLE).tcl
else
	@echo "Itcl package not installed. Example will not run. Sorry!"
endif

dogdb: $(EXAMPLE).elf
if TCL_ITCL_AVAILABLE
	../simulavr.tcl -d $(AVR_CPU) -f $(EXAMPLE).elf -u -F 271\
	    -s $(srcdir)/$(EXAMPLE).tcl -g
else
	@echo "Itcl package not installed. Example will not run. Sorry!"
endif

$(EXAMPLE).elf: hd44780.o lcd.o stdiodemo.o uart.o
	$(AVR_GCC) $(AVR_CFLAGS) -o $@ $^ -lm

.c.o:
	$(AVR_GCC) $(AVR_CFLAGS) -c -o $@ $<

hd44780.o: hd44780.c
lcd.o: lcd.c
stdiodemo.o: stdiodemo.c
uart.o: uart.c
