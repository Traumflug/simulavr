dnl Process this file with autoconf to produce a configure script.
AMDEP
AC_PREREQ(2.59)
AC_INIT([Simulavrxx], 0.8.006, [Klaus Rudolph <lts-rudolph@gmx.de>], [simulavrxx])
AC_CONFIG_SRCDIR([src/main.cpp])
AM_CONFIG_HEADER(src/config.h)
AC_CONFIG_AUX_DIR(config)

AM_INIT_AUTOMAKE
AM_MAINTAINER_MODE

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_LIBTOOL
AC_SWIG

AC_CHECK_PROG(AVR_AS, avr-as, avr-as)
AC_CHECK_PROG(AVR_LD, avr-ld, avr-ld)
AC_CHECK_PROG(AVR_GCC, avr-gcc, avr-gcc)
AC_CHECK_PROG(AVR_GXX, avr-g++, avr-g++)
AC_PATH_PROG(TCL_WISH, wish wish8.4 wish8.3 wish8.2)
AC_CHECK_PROG(CCACHE, ccache, ccache, "")

AC_SUBST([AVR_AS])
AC_SUBST([AVR_LD])
AC_SUBST([AVR_GCC])
AC_SUBST([AVR_GXX])
AC_SUBST([TCL_WISH])
AC_SUBST([CCACHE])


# Checks for libraries.
AC_CHECK_LIB([ncurses], [cbreak])

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([arpa/inet.h fcntl.h netinet/in.h stdlib.h string.h sys/socket.h unistd.h])


# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_TYPE_SIZE_T
AC_C_VOLATILE

if test -e ${swig_path}; then
 AC_MSG_NOTICE([got swig])
else
 swig_path=""
fi

if (echo $CXX | grep ccache); then
  echo
else 
  CC="$CCACHE $CC"
  CXX="$CCACHE $CXX"
fi

AC_SUBST([CXX])

AM_CONDITIONAL(HAVE_WISH, test x$TCL_WISH != x)

AC_PATH_PROGS(TCL_CONFIG, [tclConfig.sh], [""], [/usr/lib/tcl /usr/lib/tcl8.4 /usr/lib/tcl8.3 /usr/lib/tcl8.2 /usr/lib])

# find any old tcl, if present. (AC_SEARCH_LIBS more useful here? I
# think we don't want to always include it in LIBS which AC_SERCH_LIBS
# seems to do)
if test x"${TCL_CONFIG}" != "x"; then
    source ${TCL_CONFIG}
    # WAR: I don't know what I'm doing...TCL_LIB_SPEC contains
    # an unepanded variable ${TCL_DBGX} when I looked..
    MYTCL_LIB_SPEC=`eval echo "${TCL_LIB_SPEC}"`
    echo "MYTCL_LIB_SPEC=$MYTCL_LIB_SPEC"
else
    AC_MSG_WARN([Failed to find tclConfig.sh... tcl dependent code may fail to build...ask package maintainer to update configure process])
fi


AC_SUBST([TCL_INCLUDE_SPEC])
AC_SUBST([TCL_LIB_SPEC])
AC_SUBST([MYTCL_LIB_SPEC])

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_TYPE_SIGNAL
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([inet_ntoa memset socket strdup strerror])

dnl This macro searches for python version 2.1.1 or newer
AC_CACHE_CHECK([for python >= 2.1.1], [_cv_python_211], [

changequote(<@, @>)

_cv_python_211='no' ;
if python -V 2>&1 | grep -q Python ; then
  cat <<EOF > py_ver.py
import sys
v = sys.version_info
ver = (int(v[0]) << 16) + (int(v[1]) << 8) + int(v[2])
if ver < 0x020101:
        print 'no'
else:
        print 'yes'
EOF
  _cv_python_211=`python py_ver.py`
  rm -f py_ver.py
fi

changequote([, ])
])
AC_PYTHON

if test "x$_cv_python_211" = "xyes" ; then
  ac_regression_subdir="regress"
else
  AC_MSG_WARN([ ])
  AC_MSG_WARN([Python >= 2.1.1 not found.])
  AC_MSG_WARN([Regression tests will not be run.])
  AC_MSG_WARN([ ])
fi
AC_SUBST([ac_regression_subdir])
AM_CONDITIONAL(COND_HAS_PYTHON, [test "x$_cv_python_211" = "xyes"])


AC_ARG_WITH([bfd-path],
	    [--with-bfd-path=path location of AVR-binutils version of libbfd],
	    [AVR_BFD_SEARCH_STEP([${withval}])
])
if test x"$bfd_h_location" == "x"; then
  libbfd_root_location=$(which avr-readelf | sed -e ['s/\/[^\/]*\/[^\/]*$//']) 2> /dev/null
  AVR_BFD_SEARCH_STEP([${libbfd_root_location}/${host}/avr])
  AVR_BFD_SEARCH_STEP([${libbfd_root_location}])
fi

if test x"${bfd_h_location}" == x; then
AC_MSG_ERROR([

*** ERROR ***

Provide --with-bfd-path=path-to-your-avr-bfd.h to ".\configure" or ensure
your AVR cross-compiler toolset is in the path (read-elf is used to auto-detect
where your AVR-build libbfd library is).

For example, use the --enable-install-libbfd flag when configuring binutils for
the AVR or execute "make install_libbfd" from your AVR-binutils' bfd subdirectory.
Observe where the files are installed. For example:

  ./configure --with-bfd-path=/home/someuser/build-avr-binutils/bfd

This will make include/bfd.h and lib/libbfd.a available as required 
by this package.

Alternatively, you may just keep your AVR-binutils build files around
and point to the bfd subdirectory there.

GNU locate may be useful: locate bfd.h
and see if there's a bfd.h in any of
the AVR directories.

])
fi

AC_SUBST([bfd_h_location])
AC_SUBST([bfd_a_location])

echo set libbfd_root_location to ${libbfd_root_location}

AVR_LIBIBERTY([${libbfd_root_location}])
avr_readelf=$(dirname $(dirname [$(which avr-readelf])))
AVR_LIBIBERTY([${avr_readelf}/lib])


AC_SUBST([ac_aux_dir])
my_top_builddir=`pwd`/
AC_SUBST([my_top_builddir])

AC_CONFIG_FILES([Makefile
        doc/Makefile
        src/Makefile
        regress/Makefile
        regress/regress.py
        regress/modules/Makefile
        regress/test_opcodes/Makefile
        examples/Makefile examples/anacomp/Makefile
        examples/gui.tcl examples/anacomp/check.tcl
        examples/anacomp/checkdebug.tcl
        examples/anacomp/checkdebug.gdb
         ])
AC_OUTPUT

if test "x${swig_path}" == "x"; then
AC_MSG_WARN([
*** WARNING ***
swig not found, so it is not possible to build simulavr.so and tcl examples will not run. Though simulavrxx itself may be built without it.
])
fi

echo "swig_path         = " ${swig_path}
echo "libbfd.a location = " ${bfd_a_location}
echo "bfd.h location     = " ${bfd_h_location}
echo "tcl include        = " ${TCL_INCLUDE_SPEC}
echo "tcl version        = " ${TCL_VERSION}
echo "tcl lib spec       = " ${TCL_LIB_SPEC}
echo "my tcl lib spec    = " ${MYTCL_LIB_SPEC}



