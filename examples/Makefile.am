#
#  $Id$
#
#

SUBDIRS = simple_ex1 anacomp atmel_key
if USE_TCL
SUBDIRS += anacomp stdiodemo
endif

noinst_DATA = kbd.xbm

all-local: $(objdir)/kbd.xbm

$(objdir)/kbd.xbm: $(srcdir)/kbd.xbm
	test -r kbd.xbm || cp $(srcdir)/kbd.xbm kbd.xbm

EXTRA_DIST = kbd.xbm
