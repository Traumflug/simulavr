#
#  $Id$
#

AVR_LIBBFD_LIB=@AVR_LIBBFD_LIB@
AVR_LIBBFD_INC=@AVR_LIBBFD_INC@
SWIG=@SWIG@
PYTHON_CPPFLAGS=@PYTHON_CPPFLAGS@

AM_CXXFLAGS=-I$(AVR_LIBBFD_INC)

if USE_SWIG
if USE_PYTHON
LIBPYSIMULAVR=_pysimulavr.so
endif
endif

bin_PROGRAMS    = simulavr
@MAINT@ noinst_PROGRAMS = kbdgentables

lib_LTLIBRARIES = 

if USE_TCL
TCL_WRAP_SRC = simulavr_wrap.cxx
TCL_LIB      = @AVR_TCL_LIB@
TCL_INCLUDE  = @AVR_TCL_INCLUDE@
lib_LTLIBRARIES	+= libsimulavr.la
endif

if USE_VERILOG
lib_LTLIBRARIES	+= libavrvpi.la
endif

noinst_LTLIBRARIES=libsim.la

if USE_FAB
FAB_CPP=avr_ATtiny2313.cpp avr_ATtiny15.cpp avr_ATmega8.cpp
FAB_HEADER=avr_ATtiny2313.h avr_ATtiny15.h avr_ATmega8.h
endif

libsim_la_SOURCES = \
  adcpin.cpp application.cpp at4433.cpp at8515.cpp atmega48.cpp atmega128.cpp \
  $(FAB_CPP) \
  avrdevice.cpp avrerror.cpp avrfactory.cpp avrmalloc.cpp decoder.cpp \
  decoder_trace.cpp flash.cpp gdbserver.cpp hardware.cpp helper.cpp \
  hwacomp.cpp hwad.cpp hweeprom.cpp hwextirq.cpp hwmega48extirq.cpp \
  hwmegaextirq.cpp hwmegatimer0123irq.cpp hwmegatimer.cpp hwmegax8timer.cpp \
  hwmegax8timerirq.cpp hwpinchange.cpp hwport.cpp hwspi.cpp hwsreg.cpp \
  hwstack.cpp hwtimer01irq.cpp hwtimer.cpp hwuart.cpp hwwado.cpp \
  ioregs.cpp irqsystem.cpp keyboard.cpp lcd.cpp memory.cpp \
  mysocket.cpp net.cpp pin.cpp pinatport.cpp pinmon.cpp printable.cpp \
  rwmem.cpp scope.cpp serialrx.cpp serialtx.cpp spisrc.cpp spisink.cpp \
  string2.cpp systemclock.cpp trace.cpp ui.cpp 

nodist_libsimulavr_la_SOURCES = $(TCL_WRAP_SRC)

libsimulavr_la_LDFLAGS = -version-info 0:0:0 -rpath ${prefix}/lib
libsimulavr_la_LIBADD = $(libsim_la_OBJECTS) -lz -lbfd $(AVR_LIBIBERTY_LIB)

pkginclude_HEADERS = \
  adcpin.h application.h at4433.h at8515.h atmega48.h atmega128.h avrdevice.h \
  $(FAB_HEADER) \
  avrdevice_impl.h avrerror.h avrfactory.h avrmalloc.h breakpoint.h \
  config_deprecated.h decoder.h externaltype.h flash.h \
  funktor.h hwacomp.h hwad.h hweeprom.h hwextirq.h hwmega48extirq.h \
  hwmegaextirq.h hwmegatimer0123irq.h hwmegatimer.h hwmegax8timer.h \
  hwmegax8timerirq.h hwpinchange.h hwport.h hwspi.h hwsreg.h hwstack.h \
  hwtimer01irq.h hwtimer.h hwuart.h hwwado.h ioregs.h \
  irqsystem.h keyboard.h keynumber_to_scancode.dat lcd.h \
  memory.h mysocket.h net.h pin.h pinatport.h pinnotify.h pinmon.h \
  printable.h rwmem.h scope.h serialrx.h serialtx.h simulationmember.h \
  spisrc.h spisink.h systemclock.h systemclocktypes.h trace.h types.h \
  ui.h xcode_to_keynumber.dat

simulavr_SOURCES = main.cpp $(COMMON_SRC)

simulavr_LDADD = libsimulavr.la -lbfd \
   -lz $(TCL_LIB) $(AVR_LIBIBERTY_LIB)

TCLHEADER = avrdevice.h at8515.h atmega128.h at4433.h gdb.h hardware.h \
    keyboard.h lcd.h net.h pin.h keyboard.h rwmem.h scope.h serialrx.h \
    serialtx.h systemclock.h systemclocktypes.h trace.h ui.h

if USE_VERILOG
libavrvpi_la_SOURCES =	   \
	$(libsim_la_SOURCES) \
	vpi.cpp

libavrvpi_la_LDFLAGS = -version-info 0:0:0 -rpath ${prefix}/lib
libavrvpi_la_LIBADD=			\
	-L$(AVR_LIBIBERTY_INC)/../lib -lbfd $(AVR_LIBIBERTY_LIB) -lz	

all-local:	avr.vpi

avr.vpi:	libavrvpi.la
	cp .libs/libavrvpi.so avr.vpi
endif

simulavr_wrap.cxx: simulavr.i $(TCLHEADER)
	@SWIG@ -o $@ $(srcdir)/simulavr.i

#simulavr_wrap.o: simulavr_wrap.cxx
#	$(CXX) -I$(SRCDIR) -I$(TCL_INCLUDE) -c $<

simulavr_wrap.lo: simulavr_wrap.cxx
	$(LIBTOOL) --tag=CXX $(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) --mode=compile $(CXX) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(TCL_INCLUDE) $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS) -MT $@ -MD -MP -MF .deps/simulavr_wrap.Tpo -c -o $@ $<

$(srcdir)/keyboard.cpp : $(srcdir)/keytrans.h 

@MAINT@ kbdgentables_SOURCES = kbdgentables.cpp

@MAINT@ $(srcdir)/keytrans.h: $(builddir)/kbdgentables \
@MAINT@     $(srcdir)/keynumber_to_scancode.dat \
@MAINT@     $(srcdir)/xcode_to_keynumber.dat
@MAINT@		$(builddir)/kbdgentables $(srcdir)

# Python bindings
_pysimulavr.so: pysimulavr_wrap.cpp simulavr $(libsim_la_OBJECTS) $(libsim_la_DEPENDENCIES)
	$(CXXLINK) -I$(srcdir) $(PYTHON_CPPFLAGS) -fPIC -shared $< \
	    $(libsim_la_OBJECTS) $(libsim_la_LIBADD) $(LIBS) \
	    -lbfd $(LIBIBERTY) -lc -lm -lncurses \
	    -o $@ $(NonPortableDarwin_GXX_Flags)

pysimulavr_wrap.cpp: $(srcdir)/pysimulavr.i
	$(SWIG) $(SWIG_PYTHON_OPT) $(PYTHON_CPPFLAGS) -o $@ $?

EXTRA_DIST = simulavr.i pysimulavr.i vpi.cpp

MAINTAINERCLEANFILES = keytrans.h

CLEANFILES = simulavr_wrap.cxx \
   _pysimulavr.so pysimulavr.py pysimulavr_wrap.cpp avr.vpi

$(FAB_CPP) $(FAB_HEADER): fab/* fab/atmel-xml/*
	cd fab ; ./avr-fab.py	

all-am: $(LIBPYSIMULAVR)
