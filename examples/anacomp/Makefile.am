CC=ccache gcc
CXX=ccache g++
bfd_la_location = @bfd_la_location@
bfd_h_location  = @bfd_h_location@
libiberty_location = @libiberty_location@

#MAINTAINERCLEANFILES 	= Makefile.in stamp-common
MAINTAINERCLEANFILES 	= Makefile.in

pkginclude_HEADERS = 

EXTRA_DIST           = main.c

do: anacomp
	wish check.tcl

# WAR: should detect avr-toolset first!

anacomp.o : $(srcdir)/main.c
	avr-gcc -g -O2 -mmcu=at90s4433 -c -o $@ $<

anacomp : anacomp.o
	avr-gcc -mmcu=at90s4433 -o $@ $<

SUBDIRS =