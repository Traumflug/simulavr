#
#  $Id$
#
#

SUBDIRS = simple_ex1 anacomp atmel_key
if USE_TCL
SUBDIRS += anacomp stdiodemo
endif

noinst_DATA = kbd.xbm

CLEANFILES = $(builddir)/kbd.xbm

all-local: $(builddir)/kbd.xbm

$(builddir)/kbd.xbm: $(srcdir)/kbd.xbm
	test -r $(builddir)/kbd.xbm || cp $(srcdir)/kbd.xbm $(builddir)/kbd.xbm

EXTRA_DIST = kbd.xbm
