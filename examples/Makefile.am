#
#  $Id$
#
#

SUBDIRS = simple_ex1 anacomp atmel_key atmega128_timer
if USE_TCL
SUBDIRS += anacomp atmega48 spi stdiodemo feedback
endif

noinst_DATA = kbd.xbm

#CLEANFILES = $(builddir)/kbd.xbm

all-local: $(builddir)/kbd.xbm

$(builddir)/kbd.xbm: $(srcdir)/kbd.xbm
	-test -r $(builddir)/kbd.xbm || cp $(srcdir)/kbd.xbm $(builddir)/kbd.xbm

EXTRA_DIST = kbd.xbm
