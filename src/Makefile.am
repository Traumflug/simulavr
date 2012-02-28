#
#  $Id$
#

SUBDIRS = hwtimer cmd ui python

AVR_LIBBFD_LIB=@AVR_LIBBFD_LIB@
AVR_LIBBFD_INC=@AVR_LIBBFD_INC@
SWIG=@SWIG@
PYTHON_CPPFLAGS=@PYTHON_CPPFLAGS@

AM_CXXFLAGS=-I$(AVR_LIBBFD_INC)
CXXFLAGS=-g -O2 -Icmd -Iui -Ihwtimer

bin_PROGRAMS    = simulavr
@MAINT@ noinst_PROGRAMS = kbdgentables

lib_LTLIBRARIES = 
noinst_LTLIBRARIES =

if USE_SWIG

if USE_TCL
TCL_WRAP_SRC = simulavr_wrap.cxx
TCL_LIB      = @AVR_TCL_LIB@
TCL_INCLUDE  = @AVR_TCL_INCLUDE@
lib_LTLIBRARIES += libsimulavr.la
endif

if PYTHON_USE
PYTHON_MODUL_LIB=_pysimulavr@PYTHON_MODULE_EXTENSION@
PYTHON_XTRA_INC=-Ipython
PYTHON_MODUL_INTERFACE=$(srcdir)/python/pysimulavr.i
noinst_LTLIBRARIES += lib_pysimulavr.la
nodist_lib_pysimulavr_la_SOURCES=pysimulavr_wrap.cpp
lib_pysimulavr_la_CPPFLAGS=$(PYTHON_CPPFLAGS) $(PYTHON_XTRA_INC)
lib_pysimulavr_la_LDFLAGS = -version-info 0:0:0 -rpath ${prefix}/lib
lib_pysimulavr_la_LIBADD= libsim.la \
  -L$(AVR_LIBBFD_LIBPATH) -lbfd -L$(AVR_LIBIBERTY_LIBPATH) -liberty $(EXTRA_LIBS)

pysimulavr_wrap.cpp: $(PYTHON_MODUL_INTERFACE)
	$(SWIG) $(SWIG_PYTHON_OPT) $(PYTHON_XTRA_INC) -o $@ $?

$(PYTHON_MODUL_LIB): lib_pysimulavr.la
if SYS_MINGW
	$(CXX) -shared -o $(PYTHON_MODUL_LIB) \
		$(addprefix .libs/,$(lib_pysimulavr_la_OBJECTS:.lo=.o)) .libs/libsim.a \
		-lstdc++ -L$(AVR_LIBBFD_LIBPATH) -lbfd -L$(AVR_LIBIBERTY_LIBPATH) -liberty \
		$(EXTRA_LIBS) $(PYTHON_LDFLAGS)
else
	cp .libs/lib_pysimulavr.so $(PYTHON_MODUL_LIB)
endif

endif

endif

if USE_VERILOG
noinst_LTLIBRARIES += libavrvpi.la
endif

lib_LTLIBRARIES += libsim.la

if USE_FAB
FAB_CPP=avr_ATtiny2313.cpp avr_ATtiny15.cpp avr_ATmega8.cpp
FAB_HEADER=avr_ATtiny2313.h avr_ATtiny15.h avr_ATmega8.h
endif

libsim_la_SOURCES = \
  adcpin.cpp application.cpp at4433.cpp at8515.cpp atmega668base.cpp atmega128.cpp \
  at90canbase.cpp atmega8.cpp atmega1284abase.cpp \
  atmega16_32.cpp attiny2313.cpp externalirq.cpp \
  avrdevice.cpp avrerror.cpp avrfactory.cpp avrmalloc.cpp decoder.cpp \
  decoder_trace.cpp flash.cpp flashprog.cpp hardware.cpp helper.cpp cmd/gdbserver.cpp \
  hwacomp.cpp hwad.cpp hweeprom.cpp \
  hwtimer/timerprescaler.cpp hwtimer/prescalermux.cpp \
  hwtimer/timerirq.cpp hwpinchange.cpp hwport.cpp hwspi.cpp hwsreg.cpp \
  hwtimer/icapturesrc.cpp hwstack.cpp hwtimer/hwtimer.cpp hwuart.cpp hwwado.cpp \
  ioregs.cpp irqsystem.cpp ui/keyboard.cpp ui/lcd.cpp memory.cpp \
  ui/mysocket.cpp net.cpp pin.cpp ui/extpin.cpp pinatport.cpp pinmon.cpp \
  rwmem.cpp ui/scope.cpp ui/serialrx.cpp ui/serialtx.cpp spisrc.cpp spisink.cpp \
  specialmem.cpp string2.cpp systemclock.cpp traceval.cpp ui/ui.cpp 

nodist_libsim_la_SOURCES =  $(FAB_CPP)

libsim_la_LDFLAGS = -version-info 0:0:0 -rpath ${prefix}/lib

nodist_libsimulavr_la_SOURCES = $(TCL_WRAP_SRC)
libsimulavr_la_LDFLAGS = -version-info 0:0:0 -rpath ${prefix}/lib
libsimulavr_la_LIBADD = $(libsim_la_OBJECTS) -lz \
  -L$(AVR_LIBBFD_LIBPATH) -lbfd -L$(AVR_LIBIBERTY_LIBPATH) -liberty $(EXTRA_LIBS)

pkginclude_HEADERS = \
  adcpin.h application.h at4433.h at8515.h atmega128.h atmega16_32.h attiny2313.h \
  at90canbase.h atmega8.h\
  atmega668base.h atmega1284abase.h avrdevice.h externalirq.h hardware.h helper.h global.h \
  avrdevice_impl.h avrerror.h avrfactory.h avrmalloc.h breakpoint.h string2.h \
  config_deprecated.h decoder.h externaltype.h flash.h flashprog.h hwdecls.h \
  funktor.h hwacomp.h hwad.h hweeprom.h string2_template.h hwpinchange.h \
  hwport.h hwspi.h hwsreg.h hwstack.h hwuart.h hwwado.h ioregs.h irqsystem.h \
  memory.h net.h pin.h pinatport.h pinnotify.h pinmon.h printable.h rwmem.h \
  simulationmember.h spisrc.h spisink.h specialmem.h systemclock.h \
  systemclocktypes.h traceval.h types.h

nodist_pkginclude_HEADERS =  $(FAB_HEADER)

simulavr_SOURCES = cmd/main.cpp cmd/dumpargs.cpp
simulavr_LDADD = $(libsim_la_OBJECTS) $(LIBZ_FLAGS) \
  -L$(AVR_LIBBFD_LIBPATH) -lbfd -L$(AVR_LIBIBERTY_LIBPATH) -liberty \
  $(EXTRA_LIBS) $(LIBWSOCK_FLAGS)

if USE_VERILOG
VPI_LIB=avr.vpi
libavrvpi_la_SOURCES = vpi.cpp cmd/dumpargs.cpp
libavrvpi_la_DEPENDENCIES = $(libsim_la_OBJECTS)
libavrvpi_la_LDFLAGS = -version-info 0:0:0 -rpath ${prefix}/lib
libavrvpi_la_LIBADD= $(libsim_la_OBJECTS) $(LIBZ_FLAGS) \
  -L$(AVR_LIBBFD_LIBPATH) -lbfd -L$(AVR_LIBIBERTY_LIBPATH) -liberty $(EXTRA_LIBS)
libavrvpi_la_CXXFLAGS = $(AM_CXXFLAGS)

$(VPI_LIB): libavrvpi.la
	cp .libs/libavrvpi.so $(VPI_LIB)

install-vpi:
	$(INSTALL) -d $(DESTDIR)$(libdir)/ivl
	$(INSTALL_PROGRAM) $(VPI_LIB) $(DESTDIR)$(libdir)/ivl

endif

TCLHEADER = avrdevice.h at8515.h atmega128.h at4433.h cmd/gdb.h hardware.h \
    ui/keyboard.h ui/lcd.h net.h pin.h ui/extpin.h ui/keyboard.h rwmem.h \
    ui/scope.h ui/serialrx.h ui/serialtx.h systemclock.h systemclocktypes.h \
    ui/ui.h

simulavr_wrap.cxx: simulavr.i $(TCLHEADER)
	@SWIG@ -o $@ $(srcdir)/simulavr.i

simulavr_wrap.lo: simulavr_wrap.cxx
	$(LIBTOOL) --tag=CXX $(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) --mode=compile $(CXX) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(TCL_INCLUDE) $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS) -MT $@ -MD -MP -MF .deps/simulavr_wrap.Tpo -c -o $@ $<

$(srcdir)/ui/keyboard.cpp: $(srcdir)/ui/keytrans.h 

@MAINT@ kbdgentables_SOURCES = ui/kbdgentables.cpp

@MAINT@ $(srcdir)/ui/keytrans.h: $(builddir)/kbdgentables \
@MAINT@     $(srcdir)/ui/keynumber_to_scancode.dat \
@MAINT@     $(srcdir)/ui/xcode_to_keynumber.dat
@MAINT@	$(builddir)/kbdgentables $(srcdir)/ui

EXTRA_DIST = simulavr.i python/pysimulavr.i python/README

MAINTAINERCLEANFILES = ui/keytrans.h

CLEANFILES = simulavr_wrap.cxx $(PYTHON_MODUL_LIB) avr.vpi \
             pysimulavr_wrap.cpp pysimulavr_wrap.h pysimulavr.py pysimulavr.pyc

$(FAB_CPP) $(FAB_HEADER): fab/* fab/atmel-xml/*
	cd fab; ./avr-fab.py

all-local: $(VPI_LIB) $(PYTHON_MODUL_LIB)

