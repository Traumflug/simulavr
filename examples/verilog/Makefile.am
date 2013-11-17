#
#  $Id$
#
#

examplesdir = $(docdir)/examples/verilog

AVR_PROGS = toggle.elf loop.elf \
            left-unit.elf right-unit.elf \
            spi.elf \
            spi-waveforms.elf

AVRS = ../../src/verilog/

if USE_AVR_CROSS
if USE_FAB
if USE_VERILOG_TOOLS
EXAMPLE_TARGETS=$(AVR_PROGS) baretest.run loop.run spc.run spi.run
endif
endif
endif

EXTRA_DIST = README baretest.sav baretest.v right-unit.s singlepincomm.h \
    singlepincomm.s spc.sav spc.v csinglepincomm.c csinglepincomm.h left-unit.c \
    loop.c loop.sav loop.v toggle.c spi.c spi.sav spi.v spi-waveforms.c \
    spi-waveforms.sav spi-waveforms.v

SUFFIXES = .c .s .o .v .vvp .vcd

examples_DATA = $(EXTRA_DIST) $(AVR_PROGS) Makefile

example: $(EXAMPLE_TARGETS)

.c.o:
	$(AVR_GCC) -mmcu=attiny2313 -c -Os $^ -o $@

spi.o: spi.c
	$(AVR_GCC) -mmcu=atmega8 -c -Os $^ -o $@

spi.elf: spi.o
	$(AVR_GCC) -mmcu=atmega8 $^ -o $@

spi-waveforms.o: spi-waveforms.c
	$(AVR_GCC) -mmcu=atmega8 -c -Os $^ -o $@

spi-waveforms.elf: spi-waveforms.o
	$(AVR_GCC) -mmcu=atmega8 $^ -o $@

.s.o:
	$(AVR_GCC) -c -Wa,-gstabs -x assembler-with-cpp -o $@ $<

left-unit.elf: left-unit.o csinglepincomm.o

right-unit.elf: right-unit.o singlepincomm.o
	$(AVR_LD) -e _start $^ -o $@


.o.elf:
	$(AVR_GCC) -mmcu=attiny2313 $^ -o $@

.v.vvp:
	$(IVERILOG) $^ -s test -v -I. $(AVRS)/avr.v $(AVRS)/avr_ATtiny15.v $(AVRS)/avr_ATtiny2313.v $(AVRS)/avr_ATmega8.v -o $@ 

.vvp.run: $(AVR_PROGS)
	$(VVP) -M../../src -mavr $^

# this is suspicious, because dependency to .sav isn't given! Was before:
#%.view: %.vcd %.sav
.vcd.view:
if USE_GTKWAVE
	$(GTKWAVE) -a $*.sav $*.vcd &
else
	@echo "gtkwave isn't found. Can't do that!"
endif

clean-local:
	rm -f *.vvp *~ *.o $(AVR_PROGS) *.vcd avr.vpi *.trace

#
# trace VPI with:
# $ export VPI_TRACE=log.txt
#
