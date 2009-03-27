##
## $Id$
##

AC_INIT([SimulAVR], [0.9cvs], [simulavr-devel@nongnu.org])
AM_INIT_AUTOMAKE([1.10 -Wall -Werror foreign])
AC_PREREQ([2.61])
AC_CONFIG_MACRO_DIR([m4])
AM_MAINTAINER_MODE
AC_PROG_LIBTOOL
AC_PROG_CXX
AC_PROG_SWIG(1.3.17)
SWIG_ENABLE_CXX
SWIG_MULTI_MODULE_SUPPORT
SWIG_PYTHON
AM_CONDITIONAL([USE_SWIG],[test "x$SWIG" != 'x'])
AM_CONDITIONAL([USE_PYTHON],[test "x$PYTHON" != 'x'])
AC_WAR_PYTHON

## Check about the AVR cross compilation environment
AX_AVR_ENVIRONMENT

## Examples compile to avr code.  Do not build them if avr tools not installed
## NOTE: Cheap check -- only seeing if avr-gcc was found
if test X"${AVR_GCC}" = X"" ; then
  AC_MSG_WARN([])
  AC_MSG_WARN([AVR Cross tools not installed])
  AC_MSG_WARN([Not building examples])
  AC_MSG_WARN([])
  ac_have_avr_cross_tools=no
else
  ac_have_avr_cross_tools=yes
fi

AM_CONDITIONAL([USE_AVR_CROSS], [test x"${ac_have_avr_cross_tools}" = x"yes"]) 

## If ddd is not installed, then just invoked command line gdb
AC_PATH_PROG(DDD, [ddd], [/usr/bin/ddd], [no])
if test "${ddd}" = yes ; then
  DDD_WITH_ARGS=ddd --debugger
else
  DDD_WITH_ARGS=
fi
AC_SUBST([DDD_WITH_ARGS],${DDD_WITH_ARGS})

## Check for Tex to know if we build PDF
AC_CHECK_PROGS(TEX, [tex], [no])
AM_CONDITIONAL([HAVE_TEX], [test x"${TEX}" != x"no"])

## Check all about Tcl and parse command line arguments
AX_TCL_ENVIRONMENT

## We have to propagate all the configure flags we want to use when
## configuring for make distcheck
DISTCHECK_CONFIGURE_FLAGS="${DISTCHECK_CONFIGURE_FLAGS} \
  --with-bfd=${libbfd_root_location} \
  --with-libiberty=${libiberty_root_location}"
if test X"${tclconfig_root_path}" != X ; then
  DISTCHECK_CONFIGURE_FLAGS="${DISTCHECK_CONFIGURE_FLAGS} \
    --with-tclconfig=${tclconfig_root_path}"
fi
AC_SUBST(DISTCHECK_CONFIGURE_FLAGS)

## Certain files should only be generated if Tcl is available and enabled
if test x"${build_tcl_libs}" = x"yes" -a x"${HAVE_TCL}" != x"no" ; then
  AC_CONFIG_FILES(
    [examples/gui.tcl],
    [chmod +x examples/gui.tcl])
  AC_CONFIG_FILES(
    [examples/simulavr.tcl],
    [chmod +x examples/simulavr.tcl])
  AC_CONFIG_FILES(
    [examples/feedback/simfeedback.tcl],
    [chmod +x examples/feedback/simfeedback.tcl])
  # TBD: The following need to be removed once it is confirmed these examples
  # TBD: run with the Tcl simulavr framework.
  AC_CONFIG_FILES(
    [examples/stdiodemo/checkdebug.tcl],
    [chmod +x examples/stdiodemo/checkdebug.tcl])
  AC_CONFIG_FILES(
    [examples/spi/check.tcl],
    [chmod +x examples/spi/check.tcl])
  AC_CONFIG_FILES(
    [examples/atmega48/check.tcl],
    [chmod +x examples/atmega48/check.tcl])
else
  AC_MSG_WARN([])
  AC_MSG_WARN([Tcl Development Kit Not Installed])
  AC_MSG_WARN([Not building Tcl interface or examples])
  AC_MSG_WARN([])
fi

## Output all the configure generated files that are required all the time
AC_CONFIG_HEADERS([src/config.h])
AC_CONFIG_FILES(
  [regress/regress.py],
  [chmod +x regress/regress.py])
AC_CONFIG_FILES([\
  Makefile src/Makefile \
  doc/Makefile doc/config.texi \
  regress/Makefile regress/modules/Makefile regress/test_opcodes/Makefile \
  regress/avrtest/Makefile \
  examples/Makefile examples/anacomp/Makefile examples/atmega48/Makefile \
  examples/atmel_key/Makefile examples/feedback/Makefile \
  examples/simple_ex1/Makefile examples/spi/Makefile \
  examples/stdiodemo/Makefile \
])
AC_OUTPUT

echo
echo AVR_GCC=${AVR_GCC}
echo AVR_LIBBFD_INC=${AVR_LIBBFD_INC}
echo AVR_LIBBFD_LIB=${AVR_LIBBFD_LIB}
echo AVR_LIBIBERTY_LIB=${AVR_LIBIBERTY_LIB}
echo PYTHON=${PYTHON}
echo TCL_WISH=${TCL_WISH}

