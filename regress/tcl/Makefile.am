#
# $Id$
#

tcltestdir = $(srcdir)/regress/tcl

EXTRA_DIST = tcl-test.tcl main.c

if USE_AVR_CROSS

tcltest_PROG = main.elf

tcltest_DATA = $(EXTRA_DIST) $(tcltest_PROG) Makefile

tcltest: $(tcltest_PROG)

.c.o:
	$(AVR_GCC) -mmcu=atmega16 -c -Os $^ -o $@

.o.elf:
	$(AVR_GCC) -mmcu=atmega16 $^ -o $@

docheck: $(tcltest_PROG)
if USE_TCL
	(./tcl-test.tcl; if test "$$?" == "4"; then exit 0; else exit 1; fi)
else
	@echo "  Configure could not find TCL to run this test"
endif

check-local: docheck

endif

clean-local:
	rm -f main.elf

.PHONY: docheck

# EOF
