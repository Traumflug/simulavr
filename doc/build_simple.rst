Building and Installing
=======================

.. note::

  Examples in this chapter refer to a version 1.0.0, please replace this with your
  current version!
  
This chapter describes, how you can build an install simulavr from a tarball under
linux or other posix environment.

**For more informations about special options, how to build it on other platforms
and so one, how to build from git repository (for that you have to run autotools)
please read our developers guide!**

Prerequisites
-------------

In short, you need gcc, swig (for interfaces to interpreter languages and such),
make, python (at least 2.4, for regression tests, some examples, python interface),
tcl/tk (for tcl interface and some examples), doxygen (if you want to generate
api documentation), verilog (if you want to build verilog interface).

Here is a short list of tools, which are necessary to run configure script and to
build and install simulavr: (for more see manual)

- make (all not to old versions should work, known to work with 3.81, ubuntu 10.04)
- libtool (version >= 2.2, known to work with 2.2.6b, ubuntu 10.04)
- gcc (version known to work with 4.4.3, ubuntu 10.04)
- makeinfo (version known to work with 4.13, ubuntu 10.04)

Do the build
------------

Assume, that you have downloaded ``simulavr-1.0.0.tar.gz``, unpack it::

  tar zxvf simulavr-1.0.0.tar.gz
  cd simulavr-1.0.0
  
Now you are ready to run ``configure``::
  
  ./configure --prefix=/usr
  
This will create Makefiles for building simulavr commandline application only.
Prefix option determines, where simulavr will be installed, default is
``/usr/local``. If you hav installed python (at least 2.4), then you can enable
python to build python interface too::
  
  ./configure --enable-python
  
Enabeling TCL interface is a little bit more difficult, you have to locate
``tclConfig.sh`` script, which will be installed with your tcl package. Assume,
path to ``tclConfig.sh`` is ``/usr/lib/tcl8.4``::
  
  ./configure --with-tclconfig=/usr/lib/tcl8.4
  
To be able to build doxygen api (with installed doxygen)::
  
  ./configure --enable-doxygen-doc
  
Enable building verilog interface::
  
  ./configure --enable-verilog
  
You can mix this options together, if you want this. After configure you can run
``make`` and ``make istall`` as usual::

  make
  make install

This will build ``simulavr`` and, if switched on by configure options,
some extension modules and libraries. It installs simulavr itself, libraries and
some examples and the ``simulavr.info`` in documentation directory
``$prefix/share/doc/simulavr``.

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

