AVR_PROGRAMS=	toggle.elf loop.elf left-unit.elf right-unit.elf
AVRS=	../../src/verilog/

all:		$(AVR_PROGRAMS) \
		baretest.run \
		loop.run \
		spc.run

%.o:		%.c
	avr-gcc -mmcu=attiny2313 -c -Os $^ -o $@

%.o:	%.s
	avr-gcc -c -Wa,-gstabs -x assembler-with-cpp -o $@ $<

left-unit.elf:	left-unit.o csinglepincomm.o

right-unit.elf:	right-unit.o singlepincomm.o
	avr-ld -e _start $^ -o $@


%.elf:		%.o
	avr-gcc -mmcu=attiny2313 $^ -o $@

%.vvp:		%.v
	iverilog $^ -s test -v -I. $(AVRS)/avr.v $(AVRS)/avr_ATtiny15.v $(AVRS)/avr_ATtiny2313.v -o $@ 

%.run:		%.vvp
	vvp -M../../src -mavr $^

%.view:		%.vcd %.sav
	gtkwave -a $*.sav $*.vcd &
clean:
	rm -f *.vvp *~ *.o $(AVR_PROGRAMS) *.vcd avr.vpi *.trace

#
# trace VPI with:
# $ export VPI_TRACE=log.txt

#