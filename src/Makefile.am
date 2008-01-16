AVR_LIBBFD_LIB=@AVR_LIBBFD_LIB@
AVR_LIBBFD_INC=@AVR_LIBBFD_INC@
PYTHON_CPPFLAGS=@PYTHON_CPPFLAGS@

AM_CXXFLAGS=-I$(AVR_LIBBFD_INC)

AVR_LIBIBERTY_LIB=@AVR_LIBIBERTY_LIB@

if USE_SWIG
if USE_PYTHON
LIBPYSIMULAVR=_pysimulavr.so
endif
endif

bin_PROGRAMS    = simulavr kbdgentables

lib_LTLIBRARIES	= libavrsim_pp.la

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
libavrsim_pp_la_LDFLAGS = -version-info 0:0:0

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
	externaltype.h 		\
	flash.h			\
	funktor.h 		\
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

simulavr_SOURCES = main.cpp

simulavr_LDADD = libavrsim_pp.la $(AVR_LIBBFD_LIB) $(AVR_LIBIBERTY_LIB)

$(srcdir)/keyboard.cpp : keytrans.h 

kbdgentables_SOURCES = kbdgentables.cpp

keytrans.h: kbdgentables keynumber_to_scancode.dat xcode_to_keynumber.dat
	./kbdgentables

# Python bindings
_pysimulavr.so: pysimulavr_wrap.cpp simulavr
	$(CXX) -I$(srcdir) $(PYTHON_CPPFLAGS) $< $(patsubst %.lo,%.o,$(libavrsim_pp_la_OBJECTS)) $(AVR_LIBBFD_LIB) $(AVR_LIBIBERTY_LIB)  -lc -lm -lncurses -shared -o $@ $(NonPortableDarwin_GXX_Flags)

pysimulavr_wrap.cpp: $(srcdir)/pysimulavr.i
	$(SWIG) $(SWIG_PYTHON_OPT) $(PYTHON_CPPFLAGS) -o $@ $?



EXTRA_DIST           = pysimulavr.i 


DISTCLEANFILES:=${DISTCLEANFILES} simulavr.so keytrans.h  _pysimulavr.so pysimulavr.py pysimulavr_wrap.cpp
Cleanfiles:=${CLEANFILES} simulavr.so keytrans.h simulavr_wrap.cpp



all-am: $(LIBPYSIMULAVR)