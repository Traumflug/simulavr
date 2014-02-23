Building and Installing
=======================

.. note::

  Examples in this chapter refer to a version 1.0.0, please replace this with your
  current version!

Build
-----
  
simulavr uses GNU auto tools. This means that, given a tarball, for
version 1.0.0, for example, you should be able to use the following
steps to build and install simulavr::

  tar zxvf simulavr-1.0.0.tar.gz
  cd simulavr-1.0.0
  ./configure <configure options>
  make
  make install

This will build ``simulavr`` and, if switched on by configure options (see below),
some extension modules and libraries. It installs simulavr itself, libraries and
some examples and the ``simulavr.info`` in documentation directory
``$prefix/share/doc/simulavr``.

Install
-------

If you want to install ``simulavr.pdf`` too, you can do that after the normal
installation::

  make install-pdf

To install simulavr documentation as html::

  make install-html

Installing doxygen documentation is also possible, if doxygen is installed and
switched on by configure option::

  make install-doxygen

Same is possible for the verilog extension. avr.vpi will be installed in
``$prefix/lib/ivl`` if switched on by configure option::

  make install-vpi

Python interface will not be installed by ``make-install...``, because a right
installation depends on the actual python installation. To support the installation
of python module there is a ``setup.py`` in ``src`` directory::

  cd simulavr-1.0.0/src
  python setup.py install

If you want to create a egg-package from this python module, you have to install
python's setuptools package first. Then run::

  python setup.py build bdist_egg

For more possibilities on installing python interface, please see python
documentation (distutils package) and documentation for setuptools python
package.

Prerequsites
------------

Here is a list of tools, which are needed by building configure script, run
configure and run make:

- make (all not to old versions should work, known to work with 3.81, ubuntu 10.04)
- autoconf (version >= 2.63, known to work with 2.65, ubuntu 10.04)
- automake (version >= 1.10, known to work with 1.11.1, ubuntu 10.04)
- libtool (version >= 2.2, known to work with 2.2.6b, ubuntu 10.04)
- makeinfo (version known to work with 4.13, ubuntu 10.04)
- SWIG (version >= 1.3.18, known to work with 1.3.40, ubuntu 10.04), needed if you want to
  create tcl or python extensions or to run examples, **attention:** SWIG 1.3.x, not SWIG 2.0.x,
  see below!
- gcc (version known to work with 4.4.3, ubuntu 10.04)
- avr-gcc, avr-binutils, avr-libc (works with 4.3.4, 2.20, 1.6.7, ubuntu 10.04) for creating
  avr programs for examples
- tcl and tk, needed, if you want to create tcl interface, at least version 8.2 (known to
  work with 8.5)
- if you want to run tcl examples, then sometimes you need to Itcl package, which has to be
  compatible with tcl / tk version!
- python (version >= 2.4.0, known to work with 2.6.5, ubuntu 10.04), needed if you want to
  create python interface, run examples and/or ``make check``, **attention:** not python 3.x,
  see below!

**Attention:**

- On simulavr 1.0 build does run only with SWIG 1.3.x (last version is 1.3.40), SWIG 2.0.x
  is supported with upcoming version 1.1. **If you want to use SWIG 2.0.x with simulavr 1.0, then
  you have to replace the line 30 "AC_PROG_SWIG(1.3.18)" in file configure.ac with
  "AC_PROG_SWIG(2.0.0)".** Then run ``./bootstrap``, ``./configure ...`` and ``make`` again.
- Python 3 isn't supported yet. Use python 2.x, last 2.x version is 2.7.2. Will be changed
  in next versions!

Configure options
-----------------

Ideally this is all you should need to build/install simulavr. Below are some
of the configure options.

``--prefix``
  Use this option to specify the root directory to install simulavr
  to. ``/usr/local`` is the default.

``--disable-tcl``
  By default, the Tcl interface is enabled.  However, it is possible to
  build a standalone simulavr executable without Tcl.  When ``--disable-tcl``
  is specified, neither the simulator shared library not the examples
  requiring the Tcl GUI will be built.  By default, Tcl is enabled
  but if Tcl is not installed on your computer, Tcl will be automatically
  disabled.

``--with-tclconfig``
  If configure tells you it can't find ``tclConfig.sh``, try
  ``--with-tclconfig=/your/path/``.

``--enable-maintainer-mode``
  If specified on the configure command, the generated Makefiles will
  do more dependency tracking.  In particular, they will check the
  dependencies on all automake and autoconf generated files.  When
  not building in maintainer mode, the file ``src/keytrans.h`` will
  not be built or dependencies checked.

``--with-winsock``
  Specifies, where the winsock library is located. **Only used, if you want
  to build simulavr for windows with MingW environment and this library cannot be
  found. This should not occur.**

``--with-zlib``
  Specifies, where the libz library is located. Libtool want's to link against
  libz too, this library isn't used by simulavr. **Only used, if you want
  to build simulavr for windows with MingW environment and this library cannot be
  found. This should not occur.**

``--enable-doxygen-doc``
  If Doxygen is installed, you can build too a programming documentation. If you
  enable this with this option, then you can build this documentation with
  ``make doxygen-doc``. (not enabled by default)

``--enable-python``
  If Python is installed with a version younger than 2.1, then you can enable
  building the python interface. Python is also used for some tests and examples.
  If not enabled, (the default) then you can't run this tests and examples.

``--enable-verilog``
  If you have installed verilog package, then it's possible to enable building a
  verilog interface. (not enabled by default) See next chapter!

There are more options for running ``./configure``. To find out, what's
possible, see autotools documentation or try ``./configure --help``.

Hint: where to install
----------------------

I have found it useful to install my hand-configured-installed
files in one area. That way I can put the AVR-tools in my path only when
I'm working on AVR related work.  For reference, here is how I could
install AVR tools to ``/home/user/install``::

  mkdir b-binutils
  tar jxvf binutils-2.19.tar.bz2
  cd b-binutils
  ../binutils-2.19/configure --enable-install-libbfd \
     --prefix=/home/user/install --target=avr
  make && make install

Then I configure/install simulavr as follows::

  tar zxvf simulavr-@value{VERSION}.tar.gz
  cd simulavr-@value{VERSION}
  ./configure --prefix=/home/user/install
  make
  make install

How to build simulavr on MingW/Windows
--------------------------------------

.. note::
  
  Your should have experience with shell scripts, MingW on Windows, how to
  configure MingW.

* Install msys and mingw on your windows box. Further you need the following
  packages for msys/mingw: autoconf, automake, crypt, gmp, libtool, mpfr, perl,
  pthreads, w32api, zlib.
* If you want to use python interface, you need to install a python package
  and swigwin.
* Try ``autoconf --version``, if autoconf isn't found, then it could
  be that you can find autoconf-VVV (with VVV as autoconf version!) in your
  ``/mingw/bin``. If so, copy autoconf-VVV to autoconf. Same
  procedure with automake, autoheader, autom4te, aclocal!
* Unpack simulavr package or checkout/clone a simulavr repo. If you use a
  simulavr distribution package (you can find configure script), then it's high
  recommended to remove also generated files from autoconf process, run
  ``make clean && make distclean && ./bootstrap -c`` in package root.
* Run ``./bootstrap`` in package root. This will (re)build configure
  script and also all necessary files to run configure.
* Then run configure: ``./configure``
* If configure was successfull, then you cann proceed with ``make`` and
  so one ...
* If you want to use python interface and you have installed Python and SWIG,
  then you should use the following options for configure:
  ``./configure --enable-python PYTHON_LDFLAGS="-LX:/PYPATH/libs -lpython25"``
  where ``X:/PYPATH`` is **your** path to your python installations. (e.g. where the
  python.exe can be found) Replace also the name of the library (here ``python25``)
  to the right name from **your** installation, for python 2.6.x it is for example
  ``python26`` Don't use configure option ``--enable-python=X:/PYPATH/python``,
  because there is a bug in m4 scripts.
* If you get a linker error, which reports, that symbols like ``libintl_...`` are not
  found, then you have to install at first libintl-...-dev package. (part of gettext
  packages!) Then run configure again with ``EXTRA_LIBS='-L/mingw/lib -lintl'``, where the
  path ``/mingw/lib`` is the path to ``libintl.a``. This is only necessary, if this linker
  error's are shown. (depending on what version of gettext you've used)

