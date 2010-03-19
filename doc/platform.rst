Platform Related Notes
======================

Gentoo GNU/Linux distribution
-----------------------------

To install the AVR cross compiler toolchain, try: ``crossdev -t AVR``

Of course you may need to ``emerge crossdev`` first.

There have been some problems reported with crossdev. Have a look to
build scripts for Linux provided by the AVR-Freaks.  This script has
problems with the bfd-libs but the rest builds easily.

No ebuild for simulavr exists yet, but for me, the standard ./configure
&& make works. Let me know if this is not the case for you.

