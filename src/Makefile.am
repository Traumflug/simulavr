#
#  $Id$
#

AVR_LIBBFD_LIB=@AVR_LIBBFD_LIB@
AVR_LIBBFD_INC=@AVR_LIBBFD_INC@
SWIG=@SWIG@
PYTHON_CPPFLAGS=@PYTHON_CPPFLAGS@

AM_CXXFLAGS=-I$(AVR_LIBBFD_INC)

bin_PROGRAMS    = simulavr
@MAINT@ noinst_PROGRAMS = kbdgentables

lib_LTLIBRARIES = 

if USE_SWIG

if USE_TCL
TCL_WRAP_SRC = simulavr_wrap.cxx
TCL_LIB      = @AVR_TCL_LIB@
TCL_INCLUDE  = @AVR_TCL_INCLUDE@
lib_LTLIBRARIES += libsimulavr.la
endif

if PYTHON_USE
PYTHON_MODUL_LIB=_pysimulavr.so
lib_LTLIBRARIES += lib_pysimulavr.la
lib_pysimulavr_la_SOURCES=pysimulavr_wrap.cpp
lib_pysimulavr_la_CPPFLAGS=$(PYTHON_CPPFLAGS)
lib_pysimulavr_la_LDFLAGS = -version-info 0:0:0 -rpath ${prefix}/lib
lib_pysimulavr_la_LIBADD= $(libsim_la_OBJECTS) \
  -L$(AVR_LIBIBERTY_LIBPATH) -liberty -L$(AVR_LIBBFD_LIBPATH) -lbfd

pysimulavr_wrap.cpp: $(srcdir)/pysimulavr.i
	$(SWIG) $(SWIG_PYTHON_OPT) -o $@ $?

$(PYTHON_MODUL_LIB): lib_pysimulavr.la
	cp .libs/lib_pysimulavr.so $(PYTHON_MODUL_LIB)
endif

endif

if USE_VERILOG
lib_LTLIBRARIES += libavrvpi.la
endif

lib_LTLIBRARIES += libsim.la

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
  specialmem.cpp string2.cpp systemclock.cpp trace.cpp traceval.cpp ui.cpp 

libsim_la_LDFLAGS = -version-info 0:0:0 -rpath ${prefix}/lib

nodist_libsimulavr_la_SOURCES = $(TCL_WRAP_SRC)
libsimulavr_la_LDFLAGS = -version-info 0:0:0 -rpath ${prefix}/lib
libsimulavr_la_LIBADD = $(libsim_la_OBJECTS) -lz \
  -L$(AVR_LIBBFD_LIBPATH) -lbfd -L$(AVR_LIBIBERTY_LIBPATH) -liberty

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
  spisrc.h spisink.h specialmem.h systemclock.h systemclocktypes.h \
  trace.h traceval.h types.h ui.h xcode_to_keynumber.dat

simulavr_SOURCES = main.cpp
simulavr_LDADD = $(libsim_la_OBJECTS) -lz \
  -L$(AVR_LIBBFD_LIBPATH) -lbfd -L$(AVR_LIBIBERTY_LIBPATH) -liberty

if USE_VERILOG
VPI_LIB=avr.vpi
libavrvpi_la_SOURCES = vpi.cpp
libavrvpi_la_LDFLAGS = -version-info 0:0:0 -rpath ${prefix}/lib
libavrvpi_la_LIBADD= $(libsim_la_OBJECTS) -lz \
  -L$(AVR_LIBIBERTY_LIBPATH) -liberty -L$(AVR_LIBBFD_LIBPATH) -lbfd

$(VPI_LIB): libavrvpi.la
	cp .libs/libavrvpi.so $(VPI_LIB)
endif

TCLHEADER = avrdevice.h at8515.h atmega128.h at4433.h gdb.h hardware.h \
    keyboard.h lcd.h net.h pin.h keyboard.h rwmem.h scope.h serialrx.h \
    serialtx.h systemclock.h systemclocktypes.h trace.h ui.h

simulavr_wrap.cxx: simulavr.i $(TCLHEADER)
	@SWIG@ -o $@ $(srcdir)/simulavr.i

simulavr_wrap.lo: simulavr_wrap.cxx
	$(LIBTOOL) --tag=CXX $(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) --mode=compile $(CXX) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(TCL_INCLUDE) $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS) -MT $@ -MD -MP -MF .deps/simulavr_wrap.Tpo -c -o $@ $<

$(srcdir)/keyboard.cpp : $(srcdir)/keytrans.h 

@MAINT@ kbdgentables_SOURCES = kbdgentables.cpp

@MAINT@ $(srcdir)/keytrans.h: $(builddir)/kbdgentables \
@MAINT@     $(srcdir)/keynumber_to_scancode.dat \
@MAINT@     $(srcdir)/xcode_to_keynumber.dat
@MAINT@	$(builddir)/kbdgentables $(srcdir)

EXTRA_DIST = simulavr.i pysimulavr.i

MAINTAINERCLEANFILES = keytrans.h

CLEANFILES = simulavr_wrap.cxx \
   _pysimulavr.so pysimulavr.py avr.vpi

$(FAB_CPP) $(FAB_HEADER): fab/* fab/atmel-xml/*
	cd fab ; ./avr-fab.py	

all-local: $(VPI_LIB) $(PYTHON_MODUL_LIB)

