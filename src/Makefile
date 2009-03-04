#include the global configuration file
-include ../../config.h
# attention: if -DPROF is also defined here the simulation breaks
# after 1000000 steps!!!! Dont wonder about that again :-)
#PROF=-pg
#PROF=-pg -DPROF
#PROF=
#CXX=/opt/linux/bin/g++
#CC=/opt/linux/bin/gcc

#CFLAGS= -g -Wall $(PROF) -O3 -funroll-loops -fstrict-aliasing -fsched-interblock -falign-loops=16 -falign-jumps=16 -falign-functions=16 -falign-jumps-max-skip=15 -falign-loops-max-skip=15 -fomit-frame-pointer -foptimize-sibling-calls -finline-all-stringops -funit-at-a-time -funswitch-loops -ffast-math -march=i686 -mtune=pentium3 -I$(BFD)/bfd
CFLAGS= -g -Wall $(PROF) -O2 -mtune=pentium3 -I$(BFD)/bfd -I./../.. 
#CFLAGS= -g -Wall $(PROF) -O2 -mcpu=pentium3 -I$(BFD)/bfd
#CXXFLAGS = $(CFLAGS) -I/home/zfrdh/geda-install/include
CXXFLAGS = $(CFLAGS) 
# GEDA
ifeq ($(HAVE_GEDA),yes)
  CXXFLAGS += -I$(GEDA)/include
endif


OBJECTS= serialtx.o serialrx.o main.o decoder.o decoder_trace.o gdbserver.o avrmalloc.o avrerror.o rwmem.o hardware.o hwstack.o hweeprom.o avrdevice.o irqsystem.o hwtimer.o ui.o hwwado.o hwuart.o hwspi.o hwextirq.o hwtimer01irq.o hwsreg.o flash.o atmega128.o at8515.o hwmegatimer.o hwmegatimer0123irq.o helper.o systemclock.o lcd.o keyboard.o trace.o hwmegaextirq.o mysocket.o memory.o at4433.o hwacomp.o pin.o net.o hwport.o pinatport.o ioregs.o hwad.o scope.o printable.o application.o avrfactory.o

TCL_OBJECTS= serialtx.o serialrx.o main.o decoder.o decoder_trace.o gdbserver.o avrmalloc.o avrerror.o rwmem.o hardware.o hwstack.o hweeprom.o avrdevice.o irqsystem.o hwtimer.o ui.o hwwado.o hwuart.o hwspi.o hwextirq.o hwtimer01irq.o hwsreg.o flash.o atmega128.o at8515.o hwmegatimer.o hwmegatimer0123irq.o helper.o systemclock.o simulavr_wrap.o lcd.o keyboard.o trace.o hwmegaextirq.o mysocket.o memory.o at4433.o hwacomp.o pin.o net.o hwport.o pinatport.o ioregs.o hwad.o scope.o printable.o application.o avrfactory.o

TCLHEADER= serialtx.h serialrx.h avrdevice.h at8515.h atmega128.h at4433.h systemclock.h ui.h hardware.h pin.h net.h trace.h gdb.h lcd.h 




simulavr: $(OBJECTS) 
	$(CXX) $(CFLAGS) $(OBJECTS)  $(BFD)/bfd/libbfd.a $(BFD)/libiberty/libiberty.a $(ADD_LIBS) -lncurses -lc -lm -o  simulavr 

# pull in dependency info for *existing* .o files
-include $(OBJECTS:.o=.d)



# compile and generate dependency info;
# will also become command-less, prereq-less targets
#   sed:    strip the target (everything before colon)
#   sed:    remove any continuation backslashes
#   fmt -1: list words one per line
#   sed:    strip leading spaces
#   sed:    add trailing colons


%.o: %.c
	$(CC) -c $(CFLAGS) $*.c -o $*.o
	$(CC) -MM $(CFLAGS) $*.c > $*.d
	@mv -f $*.d $*.d.tmp
	@sed -e 's|.*:|$*.o:|' < $*.d.tmp > $*.d
	@sed -e 's/.*://' -e 's/\\$$//' < $*.d.tmp | fmt -1 | \
		sed -e 's/^ *//' -e 's/$$/:/' >> $*.d
	@rm -f $*.d.tmp


%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $*.cpp -o $*.o
	$(CXX) -MM $(CXXFLAGS) $*.cpp > $*.d
	@mv -f $*.d $*.d.tmp
	@sed -e 's|.*:|$*.o:|' < $*.d.tmp > $*.d
	@sed -e 's/.*://' -e 's/\\$$//' < $*.d.tmp | fmt -1 | \
		sed -e 's/^ *//' -e 's/$$/:/' >> $*.d
	@rm -f $*.d.tmp

#special targets for ui-keyboard 
keytrans.h: keynumber_to_scancode.dat xcode_to_keynumber.dat kbdgentables
	./kbdgentables

kbdgentables: kbdgentables.cpp
	$(CXX) $(CXXFLAGS) kbdgentables.cpp -o kbdgentables 

keyboard.o: keytrans.h




clean:
	rm -f *.o simulavr simulavr.so *.bin *.srec *.oo *.om *binm *wrap.c *wrap.cxx *.so tags *.o.go *_out dump *.d
	rm -f keytrans.h kbdgentables 
	rm -f gmon.out
	rm -f *.gch
	rm -f *.Plo
	rm -rf .deps
	rm -rf .libs
	rm -f cscope.out


simulavr_wrap.cxx: simulavr.i $(TCLHEADER)
	swig -c++ simulavr.i

simulavr_wrap.o: simulavr_wrap.cxx
	$(CXX) $(CXXFLAGS) simulavr_wrap.cxx -c

simulavr.so: $(TCL_OBJECTS)
	$(CXX) $(CXXFLAGS) $(TCL_OBJECTS) -ltcl$(TCL_VERSION) $(BFD)/bfd/libbfd.a $(BFD)/libiberty/libiberty.a -lc -lm -lncurses $(ADD_LIBS) -shared -o simulavr.so

BINUTILS_OBJECTS=$(BFD)/bfd
BFDOBJECTS= \
				$(BINUTILS_OBJECTS)/binary.o \
				$(BINUTILS_OBJECTS)/splay-tree.o \
				$(BINUTILS_OBJECTS)/section.o \
				$(BINUTILS_OBJECTS)/lbasename.o \
				$(BINUTILS_OBJECTS)/coffgen.o \
				$(BINUTILS_OBJECTS)/dwarf1.o \
				$(BINUTILS_OBJECTS)/dwarf2.o \
				$(BINUTILS_OBJECTS)/regex.o \
				$(BINUTILS_OBJECTS)/reloc.o \
				$(BINUTILS_OBJECTS)/spaces.o \
				$(BINUTILS_OBJECTS)/pex-common.o \
				$(BINUTILS_OBJECTS)/fnmatch.o \
				$(BINUTILS_OBJECTS)/bfd.o \
				$(BINUTILS_OBJECTS)/archive64.o \
				$(BINUTILS_OBJECTS)/elf.o \
				$(BINUTILS_OBJECTS)/hex.o \
				$(BINUTILS_OBJECTS)/getruntime.o \
				$(BINUTILS_OBJECTS)/md5.o \
				$(BINUTILS_OBJECTS)/fdmatch.o \
				$(BINUTILS_OBJECTS)/cpu-avr.o \
				$(BINUTILS_OBJECTS)/obstack.o \
				$(BINUTILS_OBJECTS)/targets.o \
				$(BINUTILS_OBJECTS)/tekhex.o \
				$(BINUTILS_OBJECTS)/floatformat.o \
				$(BINUTILS_OBJECTS)/xmalloc.o \
				$(BINUTILS_OBJECTS)/simple.o \
				$(BINUTILS_OBJECTS)/elf32.o \
				$(BINUTILS_OBJECTS)/stab-syms.o \
				$(BINUTILS_OBJECTS)/elf32-avr.o \
				$(BINUTILS_OBJECTS)/elf32-gen.o \
				$(BINUTILS_OBJECTS)/physmem.o \
				$(BINUTILS_OBJECTS)/format.o \
				$(BINUTILS_OBJECTS)/merge.o \
				$(BINUTILS_OBJECTS)/unlink-if-ordinary.o \
				$(BINUTILS_OBJECTS)/archive.o \
				$(BINUTILS_OBJECTS)/xstrerror.o \
				$(BINUTILS_OBJECTS)/stabs.o \
				$(BINUTILS_OBJECTS)/argv.o \
				$(BINUTILS_OBJECTS)/pex-one.o \
				$(BINUTILS_OBJECTS)/bfdwin.o \
				$(BINUTILS_OBJECTS)/concat.o \
				$(BINUTILS_OBJECTS)/hashtab.o \
				$(BINUTILS_OBJECTS)/elflink.o \
				$(BINUTILS_OBJECTS)/cp-demangle.o \
				$(BINUTILS_OBJECTS)/elf-strtab.o \
				$(BINUTILS_OBJECTS)/getopt.o \
				$(BINUTILS_OBJECTS)/getpwd.o \
				$(BINUTILS_OBJECTS)/make-temp-file.o \
				$(BINUTILS_OBJECTS)/hash.o \
				$(BINUTILS_OBJECTS)/ihex.o \
				$(BINUTILS_OBJECTS)/getopt1.o \
				$(BINUTILS_OBJECTS)/init.o \
				$(BINUTILS_OBJECTS)/lrealpath.o \
				$(BINUTILS_OBJECTS)/cplus-dem.o \
				$(BINUTILS_OBJECTS)/xstrndup.o \
				$(BINUTILS_OBJECTS)/partition.o \
				$(BINUTILS_OBJECTS)/fibheap.o \
				$(BINUTILS_OBJECTS)/archures.o \
				$(BINUTILS_OBJECTS)/fopen_unlocked.o \
				$(BINUTILS_OBJECTS)/sort.o \
				$(BINUTILS_OBJECTS)/srec.o \
				$(BINUTILS_OBJECTS)/syms.o \
				$(BINUTILS_OBJECTS)/ternary.o \
				$(BINUTILS_OBJECTS)/mkstemps.o \
				$(BINUTILS_OBJECTS)/corefile.o \
				$(BINUTILS_OBJECTS)/choose-temp.o \
				$(BINUTILS_OBJECTS)/bfdio.o \
				$(BINUTILS_OBJECTS)/xstrdup.o \
				$(BINUTILS_OBJECTS)/xatexit.o \
				$(BINUTILS_OBJECTS)/opncls.o \
				$(BINUTILS_OBJECTS)/dyn-string.o \
				$(BINUTILS_OBJECTS)/libbfd.o \
				$(BINUTILS_OBJECTS)/alloca.o \
				$(BINUTILS_OBJECTS)/xmemdup.o \
				$(BINUTILS_OBJECTS)/cache.o \
				$(BINUTILS_OBJECTS)/strsignal.o \
				$(BINUTILS_OBJECTS)/make-relative-prefix.o \
				$(BINUTILS_OBJECTS)/safe-ctype.o \
				$(BINUTILS_OBJECTS)/strerror.o \
				$(BINUTILS_OBJECTS)/pexecute.o \
				$(BINUTILS_OBJECTS)/objalloc.o \
				$(BINUTILS_OBJECTS)/cp-demint.o \
				$(BINUTILS_OBJECTS)/elf-eh-frame.o \
				$(BINUTILS_OBJECTS)/pex-unix.o \
				$(BINUTILS_OBJECTS)/xexit.o \
				$(BINUTILS_OBJECTS)/linker.o 

avr.vpi: $(OBJECTS) vpi.o
	iverilog-vpi  --name=avr $(OBJECTS) $(BFDOBJECTS) vpi.o -lc -lm  

