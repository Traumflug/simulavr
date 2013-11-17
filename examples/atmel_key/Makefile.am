#
#  $Id$
#

examplesdir = $(docdir)/examples/atmel_key

EXAMPLE=atmel_key
AVR_GCC=@AVR_GCC@
AVR_CPU=atmega128
AVR_CPU_CFLAGS =-mmcu=$(AVR_CPU)
AVR_CFLAGS = $(AVR_CPU_CFLAGS) -I. -DF_CPU=4000000UL \
  -Os -funsigned-char -funsigned-bitfields -fpack-struct \
  -fshort-enums -Wall -Wstrict-prototypes 

EXTRA_DIST = kb.c kb.h main.c pindefs.h scancodes.h serial.c serial.h \
  StdDefs.c StdDefs.h README Notes.txt atmel_key.tcl

SUFFIXES = .c

examples_DATA = $(EXAMPLE).elf $(EXTRA_DIST)

CLEANFILES = $(EXAMPLE).elf kb.o main.o serial.o StdDefs.o

# Frequency 250 ==> 4.0 MHz
do: $(EXAMPLE).elf
if TCL_ITCL_AVAILABLE
	../simulavr.tcl -d $(AVR_CPU) -f $(EXAMPLE).elf -u -F 250 \
	    -s $(srcdir)/$(EXAMPLE).tcl
else
	@echo "Itcl package not installed. Example will not run. Sorry!"
endif

dogdb: $(EXAMPLE).elf
if TCL_ITCL_AVAILABLE
	../simulavr.tcl -d $(AVR_CPU) -f $(EXAMPLE).elf -u -F 250 \
	    -s $(srcdir)/$(EXAMPLE).tcl -g
else
	@echo "Itcl package not installed. Example will not run. Sorry!"
endif

$(EXAMPLE).elf: kb.o main.o serial.o StdDefs.o
	$(AVR_GCC) $(AVR_CPU_CFLAGS) -o $@ $^ -lm

.c.o:
	$(AVR_GCC) $(AVR_CFLAGS) -c -o $@ $<

kb.o: kb.c
main.o: main.c
serial.o: serial.c
StdDefs.o: StdDefs.c

