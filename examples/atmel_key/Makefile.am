#
#  $Id$
#

AVR_GCC=@AVR_GCC@
AVR_CPU_CFLAGS =-mmcu=atmega128
AVR_CFLAGS = $(AVR_CPU_CFLAGS) -I. -DF_CPU=4000000UL \
  -Os -funsigned-char -funsigned-bitfields -fpack-struct \
  -fshort-enums -Wall -Wstrict-prototypes 

noinst_DATA = atmel_key.elf

CLEANFILES = atmel_key.elf kb.o main.o serial.o StdDefs.o

EXTRA_DIST = kb.c kb.h main.c pindefs.h scancodes.h serial.c serial.h \
  StdDefs.c StdDefs.h README Notes.txt Makefile.notauto checkdebug.gdb

do: atmel_key.elf
	$(TCL_WISH) check.tcl

dogdb: atmel_key.elf
	$(TCL_WISH) checkdebug.tcl

atmel_key.elf: kb.o main.o serial.o StdDefs.o
	$(AVR_GCC) $(AVR_CPU_CFLAGS) -o $@ $^ -lm

%.o: $(srcdir)/%.c
	$(AVR_GCC) $(AVR_CFLAGS) -c -o $@ $<

kb.o: kb.c
main.o: main.c
serial.o: serial.c
StdDefs.o: StdDefs.c

