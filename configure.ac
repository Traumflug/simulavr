##
## $Id$
##

AC_INIT([Simulavrxx], [0.8.007-WR_b], [simulavr-devel@nongnu.org])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_CONFIG_MACRO_DIR([m4])
AC_PROG_LIBTOOL
AC_PROG_CXX
AC_PROG_SWIG(1.3.17)
SWIG_ENABLE_CXX
SWIG_MULTI_MODULE_SUPPORT
SWIG_PYTHON
AM_CONDITIONAL([USE_SWIG],[test "x$SWIG" != 'x'])
AM_CONDITIONAL([USE_PYTHON],[test "x$PYTHON" != 'x'])
AC_WAR_PYTHON

AC_PATH_PROG(TCL_WISH, [wish], [/usr/bin/wish], [no])
test "${TCL_WISH}" = no && AC_MSG_ERROR([wish is required])

## If they don't have ddd installed, then just invoked command line gdb
AC_PATH_PROG(DDD, [ddd], [/usr/bin/ddd], [no])
if test "${ddd}" = yes ; then
  DDD_WITH_ARGS=ddd --debugger
else
  DDD_WITH_ARGS=
fi
AC_SUBST([DDD_WITH_ARGS],${DDD_WITH_ARGS})

dnl AC_CHECK_PROG(CCACHE, ccache, ccache)

AX_AVR_ENVIRONMENT

## Check if Tcl is desired
AC_MSG_CHECKING([Tcl desired])
SIMULAVRXX_ENABLE_TCL
AC_MSG_RESULT([${SIMULAVRXX_USE_TCL}])

# Check if Tcl development kit installed
if test x"${SIMULAVRXX_USE_TCL}" = x"yes" ; then
  AC_CHECK_HEADER([tcl.h],[TCL_h_found=yes],[TCL_h_found=no])
fi

AM_CONDITIONAL(
  [USE_TCL],
  [test x"${SIMULAVRXX_USE_TCL}" = x"yes" -a x"${Tcl_h_found}" = x"yes"]) 

AC_CONFIG_HEADERS([src/config.h])
AC_CONFIG_FILES(
  [examples/gui.tcl],
  [chmod +x examples/gui.tcl])
AC_CONFIG_FILES(
  [examples/anacomp/check.tcl],
  [chmod +x examples/anacomp/check.tcl])
AC_CONFIG_FILES(
  [examples/anacomp/checkdebug.tcl],
  [chmod +x examples/anacomp/checkdebug.tcl])
AC_CONFIG_FILES(
  [examples/stdiodemo/checkdebug.tcl],
  [chmod +x examples/stdiodemo/checkdebug.tcl])
AC_CONFIG_FILES(
  [regress/regress.py],
  [chmod +x regress/regress.py])
AC_CONFIG_FILES([\
  Makefile src/Makefile doc/Makefile \
  regress/Makefile regress/modules/Makefile regress/test_opcodes/Makefile \
  examples/Makefile examples/anacomp/Makefile \
  examples/simple_ex1/Makefile \
])
AC_OUTPUT

echo
echo AVR_LIBBFD_INC=${AVR_LIBBFD_INC}
echo AVR_LIBBFD_LIB=${AVR_LIBBFD_LIB}
echo AVR_LIBIBERTY_LIB=${AVR_LIBIBERTY_LIB}
echo PYTHON=${PYTHON}
echo TCL_WISH=${TCL_WISH}

