CC=ccache gcc
CXX=ccache g++

bfd_la_location = @bfd_la_location@
bfd_h_location  = @bfd_h_location@
libiberty_location = @libiberty_location@

MAINTAINERCLEANFILES 	= Makefile.in

EXTRA_DIST           = simulavr.i

lib_LTLIBRARIES		= libavrsim_pp.la

bin_PROGRAMS = simulavr kbdgentables

libavrsim_pp_la_SOURCES	= 	\
	ass_do.cpp      	\
	ass_do_trace.cpp	\
	at4433.cpp		\
	at8515.cpp		\
	at8515special.cpp	\
	atmega128.cpp		\
	avrdevice.cpp		\
	avrerror.cpp		\
	avrmalloc.cpp		\
	decoder.cpp		\
	flash.cpp		\
	gdbserver.cpp		\
	hardware.cpp		\
	helper.cpp		\
	hwad.cpp		\
	hwacomp.cpp		\
	hweeprom.cpp		\
	hwextirq.cpp		\
	hwmegaextirq.cpp	\
	hwmegatimer.cpp		\
	hwmegatimer0123irq.cpp	\
	hwport.cpp		\
	hwspi.cpp		\
	hwsreg.cpp		\
	hwstack.cpp		\
	hwtimer.cpp		\
	hwtimer01irq.cpp	\
	hwuart.cpp		\
	hwwado.cpp		\
	ioregs.cpp		\
	irqsystem.cpp		\
	keyboard.cpp		\
	lcd.cpp			\
	memory.cpp		\
	mysocket.cpp		\
	net.cpp			\
	pin.cpp			\
	pinatport.cpp		\
	rwmem.cpp		\
	scope.cpp		\
	systemclock.cpp		\
	trace.cpp		\
	ui.cpp 			

pkginclude_HEADERS = 	\
	ass_do.h      		\
	ass_do_trace.h		\
	at4433.h		\
	at8515.h		\
	at8515special.h		\
	atmega128.h		\
	avrdevice.h		\
	avrerror.h		\
	avrmalloc.h		\
	config.h		\
	decoder.h		\
	flash.h			\
	hardware.h		\
	helper.h		\
	hwad.h			\
	hwacomp.h		\
	hweeprom.h		\
	hwextirq.h		\
	hwmegaextirq.h		\
	hwmegatimer.h		\
	hwmegatimer0123irq.h	\
	hwport.h		\
	hwspi.h			\
	hwsreg.h		\
	hwstack.h		\
	hwtimer.h		\
	hwtimer01irq.h		\
	hwuart.h		\
	hwwado.h		\
	ioregs.h		\
	irqsystem.h		\
	keyboard.h		\
	lcd.h			\
	memory.h		\
	mysocket.h		\
	net.h			\
	pin.h			\
	pinatport.h		\
	rwmem.h			\
	scope.h			\
	systemclock.h		\
	trace.h			\
	types.h			\
	ui.h			\
	simulationmember.h breakpoint.h config_deprecated.h \
	global.h avrdevice_impl.h gdb.h keynumber_to_scancode.dat  xcode_to_keynumber.dat 

# WAR: Hmm...how do we auto-depend on this generated file? Seems we
# should be able to auto-generate at least first-generation include
# dependencies whether or not those files exist...;-( 
$(srcdir)/keyboard.cpp : keytrans.h 

keytrans.h: kbdgentables keynumber_to_scancode.dat xcode_to_keynumber.dat
	./kbdgentables

kbdgentables_SOURCES = kbdgentables.cpp

simulavr_SOURCES = main.cpp


# WAR: simulavr.so/.dll needs tp be properly integrated via the
# autotools, but I don't "get" it yet...so this hack gets us running
# at least on Linux

simulavr_DEPENDENCIES = libavrsim_pp.la

simulavr_LDADD = $(top_builddir)/src/libavrsim_pp.la $(bfd_la_location)/libbfd.a $(libiberty_location)/libiberty.a

AM_CXXFLAGS=-O2

INCLUDES		= -I$(bfd_h_location) -I$(top_srcdir)/src

# WAR:: should not build if no swig is detected...

# This is a cheat! Hopefully it works ok...
all-am: simulavr.so

simulavr_wrap.cpp: $(srcdir)/simulavr.i
	swig -c++ -o $@ $<

simulavr.so: simulavr_wrap.o simulavr
	g++ -I$(srcdir) $< $(patsubst %.lo,%.o,$(libavrsim_pp_la_OBJECTS)) -l$(LIBTCL_NAME) $(bfd_la_location)/libbfd.a $(libiberty_location)/libiberty.a -lc -lm -lncurses -shared -o simulavr.so

clean : clean_simulavr.so