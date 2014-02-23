Platform Related Notes
======================

Gentoo GNU/Linux distribution
-----------------------------

To install the AVR cross compiler toolchain, try: ``crossdev -t AVR``

Of course you may need to ``emerge crossdev`` first.

There have been some problems reported with crossdev. Have a look to
build scripts for Linux provided by the AVR-Freaks.

No ebuild for simulavr exists yet, but for me, the standard ./configure
&& make works. Let me know if this is not the case for you.

Ubuntu/Debian
-------------

On Ubuntu (as example Ubuntu 12.04) gcc, make and python are installed
by default. So, if you want to build simulavr from a distribution package
(simulavr-*.tar.gz) without script modules / verilog module you have to
install the following packages:

- g++
- texinfo

If you want to build it from the current development state, you need additional
the following packages:

- git-core
- automake (this installs automatically autoconf)
- libtool

Then you can clone the development repo (with the master branch) and build
it as described::

  > git clone git://git.savannah.nongnu.org/simulavr.git
  > cd simulavr
  > ./bootstrap
  > ./configure
  > make

