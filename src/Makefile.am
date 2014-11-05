#
#  $Id$
#

SUBDIRS = hwtimer cmd ui .

if USE_SWIG

if PYTHON_USE

SUBDIRS += python

endif

endif

AM_CXXFLAGS=-Ielfio -g -O2 -Icmd -Iui -Ihwtimer

bin_PROGRAMS    = simulavr
@MAINT@ noinst_PROGRAMS = kbdgentables

lib_LTLIBRARIES =
noinst_LTLIBRARIES =

tcldir = $(libdir)/tcl
tcl_LTLIBRARIES =
ivldir = $(libdir)/ivl
ivl_LTLIBRARIES =

if USE_SWIG

if USE_TCL
TCL_WRAP_SRC = simulavr_wrap.cxx
TCL_LIB      = @AVR_TCL_LIB@
TCL_INCLUDE  = @AVR_TCL_INCLUDE@
tcl_LTLIBRARIES += libsimulavr.la
endif

endif

if USE_VERILOG
ivl_LTLIBRARIES += avr.vpi.la
endif

lib_LTLIBRARIES += libsim.la

libsim_la_SOURCES = \
  at4433.cpp at8515.cpp atmega668base.cpp atmega128.cpp at90canbase.cpp \
  atmega8.cpp atmega1284abase.cpp attiny25_45_85.cpp atmega16_32.cpp \
  attiny2313.cpp adcpin.cpp application.cpp externalirq.cpp \
  avrdevice.cpp avrerror.cpp avrfactory.cpp avrmalloc.cpp decoder.cpp \
  decoder_trace.cpp flash.cpp flashprog.cpp hardware.cpp helper.cpp cmd/gdbserver.cpp \
  hwacomp.cpp hwad.cpp hweeprom.cpp avrsignature.cpp avrreadelf.cpp cmd/dumpargs.cpp \
  hwtimer/timerprescaler.cpp hwtimer/prescalermux.cpp \
  hwtimer/timerirq.cpp hwpinchange.cpp hwport.cpp hwspi.cpp hwsreg.cpp \
  hwtimer/icapturesrc.cpp hwstack.cpp hwtimer/hwtimer.cpp hwuart.cpp hwwado.cpp \
  ioregs.cpp irqsystem.cpp ui/keyboard.cpp ui/lcd.cpp memory.cpp \
  ui/mysocket.cpp net.cpp pin.cpp ui/extpin.cpp pinatport.cpp pinmon.cpp \
  rwmem.cpp ui/scope.cpp ui/serialrx.cpp ui/serialtx.cpp spisrc.cpp spisink.cpp \
  specialmem.cpp string2.cpp systemclock.cpp traceval.cpp ui/ui.cpp 

libsim_la_LDFLAGS = -shared -avoid-version -rpath $(libdir)
libsim_la_LIBADD = $(LIBWSOCK_FLAGS)
if SYS_MINGW
libsim_la_LDFLAGS += -no-undefined
endif

nodist_libsimulavr_la_SOURCES = $(TCL_WRAP_SRC)
libsimulavr_la_LDFLAGS = -module -avoid-version -shared
libsimulavr_la_LIBADD = libsim.la $(LIBZ_FLAGS) $(EXTRA_LIBS)
if SYS_MINGW
libsimulavr_la_LDFLAGS += -no-undefined
libsimulavr_la_LIBADD += $(TCL_LIB)
endif

pkginclude_HEADERS = \
  adcpin.h application.h at4433.h at8515.h atmega128.h atmega16_32.h attiny2313.h \
  at90canbase.h atmega8.h attiny25_45_85.h atmega668base.h atmega1284abase.h avrdevice.h \
  externalirq.h hardware.h helper.h avrdevice_impl.h avrerror.h avrfactory.h avrmalloc.h \
  string2.h decoder.h externaltype.h flash.h flashprog.h hwdecls.h \
  funktor.h hwacomp.h hwad.h hweeprom.h string2_template.h hwpinchange.h \
  hwport.h hwspi.h hwsreg.h hwstack.h hwuart.h hwwado.h ioregs.h irqsystem.h \
  memory.h net.h pin.h pinatport.h pinnotify.h pinmon.h printable.h rwmem.h \
  simulationmember.h spisrc.h spisink.h specialmem.h systemclock.h \
  systemclocktypes.h traceval.h types.h avrsignature.h avrreadelf.h \
  elfio/elfio/elf_types.hpp elfio/elfio/elfio.hpp elfio/elfio/elfio_dump.hpp \
  elfio/elfio/elfio_dynamic.hpp elfio/elfio/elfio_header.hpp elfio/elfio/elfio_note.hpp \
  elfio/elfio/elfio_relocation.hpp elfio/elfio/elfio_section.hpp \
  elfio/elfio/elfio_segment.hpp elfio/elfio/elfio_strings.hpp \
  elfio/elfio/elfio_symbols.hpp elfio/elfio/elfio_utils.hpp

export LIBSIM_SRCS=$(libsim_la_SOURCES)
export LIBSIM_HDRS=$(pkginclude_HEADERS)

simulavr_SOURCES = cmd/main.cpp
simulavr_LDADD = libsim.la $(LIBZ_FLAGS) $(EXTRA_LIBS)

if USE_VERILOG
VPI_LIB=avr.vpi
avr_vpi_la_SOURCES = vpi.cpp
avr_vpi_la_LDFLAGS = -avoid-version -module -shared
avr_vpi_la_LIBADD= libsim.la $(LIBZ_FLAGS) $(EXTRA_LIBS)
avr_vpi_la_CXXFLAGS = $(AM_CXXFLAGS)

$(VPI_LIB): avr.vpi.la
	cp .libs/avr.vpi.so $(VPI_LIB)

install-vpi:
	$(INSTALL) -d $(DESTDIR)$(ivldir)
	$(INSTALL_PROGRAM) $(VPI_LIB) $(DESTDIR)$(ivldir)

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

EXTRA_DIST = simulavr.i elfio/AUTHORS elfio/COPYING elfio/README elfio/VERSION

MAINTAINERCLEANFILES = ui/keytrans.h

CLEANFILES = simulavr_wrap.cxx $(VPI_LIB)

all-local: $(VPI_LIB)

