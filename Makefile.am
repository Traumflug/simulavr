#
# $Id$
#

ACLOCAL_AMFLAGS = -I m4

if USE_AVR_CROSS
EXAMPLES = examples
endif

SUBDIRS = src $(EXAMPLES) doc regress

EXTRA_DIST = bootstrap make_tarball README.gdb SUPPORT \
 m4/README m4/avr_tcl.m4 m4/avr_swig.m4
 
doxygen-doc:
if DX_COND_doc
	$(MAKE) -C doc doxygen-doc
else
	@echo "doxygen documentation not available! Sorry."
endif

.PHONY: doxygen-doc

