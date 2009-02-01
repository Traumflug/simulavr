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
CXXFLAGS = $(CFLAGS) -I/home/zfrdh/geda-install/include


OBJECTS= serialtx.o serialrx.o main.o decoder.o decoder_trace.o gdbserver.o avrmalloc.o avrerror.o rwmem.o hardware.o hwstack.o hweeprom.o avrdevice.o irqsystem.o hwtimer.o ui.o hwwado.o hwuart.o hwspi.o hwextirq.o hwtimer01irq.o hwsreg.o flash.o atmega128.o at8515.o hwmegatimer.o hwmegatimer0123irq.o helper.o systemclock.o lcd.o keyboard.o trace.o hwmegaextirq.o mysocket.o memory.o at4433.o hwacomp.o pin.o net.o hwport.o pinatport.o ioregs.o hwad.o scope.o printable.o application.o avrfactory.o

TCL_OBJECTS= serialtx.o serialrx.o main.o decoder.o decoder_trace.o gdbserver.o avrmalloc.o avrerror.o rwmem.o hardware.o hwstack.o hweeprom.o avrdevice.o irqsystem.o hwtimer.o ui.o hwwado.o hwuart.o hwspi.o hwextirq.o hwtimer01irq.o hwsreg.o flash.o atmega128.o at8515.o hwmegatimer.o hwmegatimer0123irq.o helper.o systemclock.o simulavr_wrap.o lcd.o keyboard.o trace.o hwmegaextirq.o mysocket.o memory.o at4433.o hwacomp.o pin.o net.o hwport.o pinatport.o ioregs.o hwad.o scope.o printable.o application.o avrfactory.o

TCLHEADER= serialtx.h serialrx.h avrdevice.h at8515.h atmega128.h at4433.h systemclock.h ui.h hardware.h pin.h net.h trace.h gdb.h lcd.h 




simulavr: $(OBJECTS) 
	$(CXX) $(CFLAGS) $(OBJECTS)  $(BFD)/bfd/libbfd.a $(BFD)/libiberty/libiberty.a -lncurses -lc -lm -o  simulavr 

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
	$(CXX) $(CXXFLAGS) $(TCL_OBJECTS) -ltcl$(TCL_VERSION) $(BFD)/bfd/libbfd.a $(BFD)/libiberty/libiberty.a -lc -lm -lncurses -shared -o simulavr.so

BFDOBJECTS= /home/zfrdh/binutils_tmp/binary.o \
	/home/zfrdh/binutils_tmp/splay-tree.o \
	/home/zfrdh/binutils_tmp/section.o \
	/home/zfrdh/binutils_tmp/lbasename.o \
	/home/zfrdh/binutils_tmp/coffgen.o \
	/home/zfrdh/binutils_tmp/dwarf1.o \
	/home/zfrdh/binutils_tmp/dwarf2.o \
	/home/zfrdh/binutils_tmp/regex.o \
	/home/zfrdh/binutils_tmp/reloc.o \
	/home/zfrdh/binutils_tmp/spaces.o \
	/home/zfrdh/binutils_tmp/pex-common.o \
	/home/zfrdh/binutils_tmp/fnmatch.o \
	/home/zfrdh/binutils_tmp/bfd.o \
	/home/zfrdh/binutils_tmp/archive64.o \
	/home/zfrdh/binutils_tmp/elf.o \
	/home/zfrdh/binutils_tmp/hex.o \
	/home/zfrdh/binutils_tmp/getruntime.o \
	/home/zfrdh/binutils_tmp/md5.o \
	/home/zfrdh/binutils_tmp/fdmatch.o \
	/home/zfrdh/binutils_tmp/cpu-avr.o \
	/home/zfrdh/binutils_tmp/obstack.o \
	/home/zfrdh/binutils_tmp/targets.o \
	/home/zfrdh/binutils_tmp/tekhex.o \
	/home/zfrdh/binutils_tmp/floatformat.o \
	/home/zfrdh/binutils_tmp/xmalloc.o \
	/home/zfrdh/binutils_tmp/simple.o \
	/home/zfrdh/binutils_tmp/elf32.o \
	/home/zfrdh/binutils_tmp/stab-syms.o \
	/home/zfrdh/binutils_tmp/elf32-avr.o \
	/home/zfrdh/binutils_tmp/elf32-gen.o \
	/home/zfrdh/binutils_tmp/physmem.o \
	/home/zfrdh/binutils_tmp/format.o \
	/home/zfrdh/binutils_tmp/merge.o \
	/home/zfrdh/binutils_tmp/unlink-if-ordinary.o \
	/home/zfrdh/binutils_tmp/archive.o \
	/home/zfrdh/binutils_tmp/xstrerror.o \
	/home/zfrdh/binutils_tmp/stabs.o \
	/home/zfrdh/binutils_tmp/argv.o \
	/home/zfrdh/binutils_tmp/pex-one.o \
	/home/zfrdh/binutils_tmp/bfdwin.o \
	/home/zfrdh/binutils_tmp/concat.o \
	/home/zfrdh/binutils_tmp/hashtab.o \
	/home/zfrdh/binutils_tmp/elflink.o \
	/home/zfrdh/binutils_tmp/cp-demangle.o \
	/home/zfrdh/binutils_tmp/elf-strtab.o \
	/home/zfrdh/binutils_tmp/getopt.o \
	/home/zfrdh/binutils_tmp/getpwd.o \
	/home/zfrdh/binutils_tmp/make-temp-file.o \
	/home/zfrdh/binutils_tmp/hash.o \
	/home/zfrdh/binutils_tmp/ihex.o \
	/home/zfrdh/binutils_tmp/getopt1.o \
	/home/zfrdh/binutils_tmp/init.o \
	/home/zfrdh/binutils_tmp/lrealpath.o \
	/home/zfrdh/binutils_tmp/cplus-dem.o \
	/home/zfrdh/binutils_tmp/xstrndup.o \
	/home/zfrdh/binutils_tmp/partition.o \
	/home/zfrdh/binutils_tmp/fibheap.o \
	/home/zfrdh/binutils_tmp/archures.o \
	/home/zfrdh/binutils_tmp/fopen_unlocked.o \
	/home/zfrdh/binutils_tmp/sort.o \
	/home/zfrdh/binutils_tmp/srec.o \
	/home/zfrdh/binutils_tmp/syms.o \
	/home/zfrdh/binutils_tmp/ternary.o \
	/home/zfrdh/binutils_tmp/mkstemps.o \
	/home/zfrdh/binutils_tmp/corefile.o \
	/home/zfrdh/binutils_tmp/choose-temp.o \
	/home/zfrdh/binutils_tmp/bfdio.o \
	/home/zfrdh/binutils_tmp/xstrdup.o \
	/home/zfrdh/binutils_tmp/xatexit.o \
	/home/zfrdh/binutils_tmp/opncls.o \
	/home/zfrdh/binutils_tmp/dyn-string.o \
	/home/zfrdh/binutils_tmp/libbfd.o \
	/home/zfrdh/binutils_tmp/alloca.o \
	/home/zfrdh/binutils_tmp/xmemdup.o \
	/home/zfrdh/binutils_tmp/cache.o \
	/home/zfrdh/binutils_tmp/strsignal.o \
	/home/zfrdh/binutils_tmp/make-relative-prefix.o \
	/home/zfrdh/binutils_tmp/safe-ctype.o \
	/home/zfrdh/binutils_tmp/strerror.o \
	/home/zfrdh/binutils_tmp/pexecute.o \
	/home/zfrdh/binutils_tmp/objalloc.o \
	/home/zfrdh/binutils_tmp/cp-demint.o \
	/home/zfrdh/binutils_tmp/elf-eh-frame.o \
	/home/zfrdh/binutils_tmp/pex-unix.o \
	/home/zfrdh/binutils_tmp/xexit.o \
	/home/zfrdh/binutils_tmp/linker.o 

avr.vpi: $(OBJECTS) vpi.o
	iverilog-vpi  --name=avr $(OBJECTS) $(BFDOBJECTS) vpi.o -lc -lm  

