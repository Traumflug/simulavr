bfd_a_location = @bfd_a_location@
bfd_h_location  = @bfd_h_location@
libiberty_location = @libiberty_location@
swig_path = @swig_path@
CCACHE = @CCACHE@
MYTCL_LIB_SPEC=@MYTCL_LIB_SPEC@
TCL_INCLUDE_SPEC=@TCL_INCLUDE_SPEC@

MAINTAINERCLEANFILES 	= Makefile.in

DISTCLEANFILES:=${DISTCLEANFILES} simulavr.so keytrans.h simulavr_wrap.cpp

EXTRA_DIST           = simulavr.i 

SUBDIRS = python
DIST_SUBDIRS = python

lib_LTLIBRARIES		= libavrsim_pp.la

bin_PROGRAMS = simulavr kbdgentables

libavrsim_pp_la_SOURCES	= 	\
	application.cpp 	\
	at4433.cpp		\
	at8515.cpp		\
	atmega128.cpp		\
	avrdevice.cpp		\
	avrerror.cpp		\
	avrmalloc.cpp		\
	decoder.cpp		\
	decoder_trace.cpp 	\
	flash.cpp		\
	gdbserver.cpp		\
	hardware.cpp		\
	helper.cpp		\
	hwacomp.cpp		\
	hwad.cpp		\
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
	printable.cpp 		\
	rwmem.cpp		\
	scope.cpp		\
	serialrx.cpp		\
	serialtx.cpp		\
	systemclock.cpp		\
	trace.cpp		\
	ui.cpp 			

pkginclude_HEADERS = 	\
	application.h \
	at4433.h		\
	at8515.h		\
	atmega128.h		\
	avrdevice.h		\
	avrdevice_impl.h	\
	avrerror.h		\
	avrmalloc.h		\
	breakpoint.h		\
	config_deprecated.h 	\
	decoder.h		\
	externaltype.h \
	flash.h			\
	funktor.h \
	gdb.h			\
	global.h		\
	hardware.h		\
	helper.h		\
	hwacomp.h		\
	hwad.h			\
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
	keynumber_to_scancode.dat	\
	lcd.h			\
	memory.h		\
	mysocket.h		\
	net.h			\
	pin.h			\
	pinatport.h		\
	pinnotify.h		\
	printable.h 		\
	rwmem.h			\
	scope.h			\
	serialrx.h		\
	serialtx.h		\
	simulationmember.h	\
	systemclock.h		\
	systemclocktypes.h	\
	trace.h			\
	types.h			\
	ui.h			\
	xcode_to_keynumber.dat

LTCXXCOMPILE = $(LIBTOOL) --tag=CXX --mode=compile $(CXX) $(DEFS) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS)
CXXLINK = $(LIBTOOL) --mode=link --tag=CXX $(CXXLD) $(AM_CXXFLAGS) $(CXXFLAGS) \
	$(AM_LDFLAGS) $(LDFLAGS) -o $@

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

simulavr_LDADD = $(top_builddir)/src/libavrsim_pp.la $(bfd_a_location)/libbfd.a $(libiberty_location)/libiberty.a

AM_CPPFLAGS=$(TCL_INCLUDE_SPEC)
AM_CXXFLAGS=-O2

INCLUDES		= -I$(bfd_h_location) -I$(top_srcdir)/src -I $(top_builddir)/src

# WAR:: should not build if no swig is detected...

# This is a cheat! Hopefully it works ok...
all-am: simulavr.so python/_simulavr.so

# TCL bindings
simulavr_wrap.cpp: $(srcdir)/simulavr.i
	$(swig_path) -c++ -o $@ $<

simulavr.so: simulavr_wrap.o simulavr
	g++ -I$(srcdir) $< $(patsubst %.lo,%.o,$(libavrsim_pp_la_OBJECTS)) $(MYTCL_LIB_SPEC) $(bfd_a_location)/libbfd.a $(libiberty_location)/libiberty.a -lc -lm -lncurses -shared -o simulavr.so

clean_simulavr.so:
	$(RM) simulavr.so 


# Python bindings
python/simulavr_wrap.cpp: $(srcdir)/simulavr.i
	$(swig_path) -c++ -python -outdir python/ -o $@ $?

python/_simulavr.so: python/simulavr_wrap.cpp simulavr
	g++ -I$(srcdir) -I/usr/include/python2.3 $< $(patsubst %.lo,%.o,$(libavrsim_pp_la_OBJECTS)) $(bfd_a_location)/libbfd.a $(libiberty_location)/libiberty.a -lc -lm -lncurses -shared -o $@

clean_python_simulavr:
	$(RM) python/_simulavr.so python/simulavr.py python/simulavr_wrap.cpp


clean : clean_simulavr.so clean_python_simulavr
