##
## $Id$
##

####
# common initialisation of autotools
####
AC_INIT([SimulAVR], [0.9cvs], [simulavr-devel@nongnu.org])
AM_INIT_AUTOMAKE([1.10 -Wall -Werror foreign])
AC_PREREQ([2.61])
AC_CONFIG_MACRO_DIR([m4])
AM_MAINTAINER_MODE

####
# check for OS and build system: MSYS/MingW
####
AC_SYS_CHECK_MINGW

####
# support of libtool, g++, swig (for tcl module and python module used)
####
AC_PROG_LIBTOOL
AC_PROG_CXX
AC_PROG_SWIG(1.3.17)
SWIG_ENABLE_CXX
SWIG_MULTI_MODULE_SUPPORT
AM_CONDITIONAL([USE_SWIG],[test "x$SWIG" != 'x'])

####
# tcl/tk support
####
## Check all about Tcl and parse command line arguments
AX_TCL_ENVIRONMENT

####
# python support, used for regression test and python module
####
AZ_PYTHON_DEFAULT
AZ_PYTHON_ENABLE
AZ_PYTHON_VERSION_ENSURE([2.1.1])
SWIG_PYTHON

####
# Check about the AVR cross compilation environment, check for libz (only on msys,
# used automaticaly by libtool!), libbfd and libiberty
####
AX_AVR_ENVIRONMENT
if test "$ac_sys_check_uname_o" = "Msys"; then
  # need libwsock32 too
  AVR_CHECK_WINSOCK
  # libtool links to libz by default, but on MSYS/MingW this is on other place
  AVR_CHECK_ZLIB_MSYS
fi

## Check for the XML files in src/fab/atmel-xml. This just checks for the
## existence of the file ATmega8.xml and assumes that all files have been placed into
## this directory.
AC_MSG_CHECKING([XML part description files in src/fab/atmel-xml/])
AM_CONDITIONAL([USE_FAB], [test -f "${srcdir}/src/fab/atmel-xml/ATmega8.xml"])
if test X"${USE_FAB_FALSE}" == X"" ; then
   AC_MSG_RESULT([NOT available])
else
   AC_MSG_RESULT([available])
fi

## Examples compile to avr code.  Do not build them if avr tools not installed
## NOTE: Cheap check -- only seeing if avr-gcc was found
if test X"${AVR_GCC}" = X"" ; then
  AC_MSG_WARN([])
  AC_MSG_WARN([AVR Cross tools not installed])
  AC_MSG_WARN([Not building examples and do tests])
  AC_MSG_WARN([])
  ac_have_avr_cross_tools=no
else
  ac_have_avr_cross_tools=yes
fi

AM_CONDITIONAL([USE_AVR_CROSS], [test x"${ac_have_avr_cross_tools}" = x"yes"])

####
# support verilog interface to build avr.vpi
####
AC_CHECK_HEADER([vpi_user.h],
    [AC_DEFINE(HAVE_VERILOG, [1], Icarus verilog interface)
        WE_HAVE_VERILOG="yes"],  
    [])
AC_ARG_ENABLE([verilog],
    [AS_HELP_STRING([--enable-verilog],[enables build of verilog modul avr.vpi])],
    [test "$WE_HAVE_VERILOG" = "yes" && AVR_BUILD_VERILOG="yes"],
    [AVR_BUILD_VERILOG="no"])
AM_CONDITIONAL([USE_VERILOG], [test "$AVR_BUILD_VERILOG" = "yes"])
AC_PATH_PROG([GTKWAVE], [gtkwave])
AM_CONDITIONAL([USE_GTKWAVE],[test "x$GTKWAVE" != "x"])
AC_PATH_PROG([IVERILOG], [iverilog])
AC_PATH_PROG([VVP], [vvp])
AM_CONDITIONAL([USE_VERILOG_TOOLS],[test "x$IVERILOG" != "x" -a "x$VVP" != "x"])

####
# support for some documenatation tools, texinfo and doxygen
####
AC_CHECK_PROGS(TEX, [tex], [no])
AM_CONDITIONAL([HAVE_TEX], [test x"${TEX}" != x"no"])
DX_DOXYGEN_FEATURE(OFF)
DX_PDF_FEATURE(OFF)
DX_PS_FEATURE(OFF)
DX_INIT_DOXYGEN([Simulavr], [simulavrxx.doxy], )

####
# definitions for distcheck
####
## We have to propagate all the configure flags we want to use when
## configuring for make distcheck
DISTCHECK_CONFIGURE_FLAGS="${DISTCHECK_CONFIGURE_FLAGS} \
  --with-bfd=${libbfd_root_location} \
  --with-libiberty=${libiberty_root_location}"
if test X"${tclconfig_root_path}" != X ; then
  DISTCHECK_CONFIGURE_FLAGS="${DISTCHECK_CONFIGURE_FLAGS} \
    --with-tclconfig=${tclconfig_root_path}"
fi
AC_SUBST([DISTCHECK_CONFIGURE_FLAGS])

####
# specify files to create
####
## Output all the configure generated files that are required all the time
AC_CONFIG_HEADERS([src/config.h])
AC_CONFIG_FILES(
  [regress/regress.py],
  [chmod +x regress/regress.py])
AC_CONFIG_FILES([
  Makefile src/Makefile
  doc/Makefile doc/config.texi
  regress/Makefile regress/modules/Makefile regress/test_opcodes/Makefile
  regress/avrtest/Makefile regress/timertest/Makefile
  examples/Makefile examples/anacomp/Makefile examples/atmega48/Makefile
  examples/atmega128_timer/Makefile examples/atmel_key/Makefile
  examples/feedback/Makefile examples/simple_ex1/Makefile
  examples/spi/Makefile examples/stdiodemo/Makefile examples/python/Makefile
])

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

####
# generate files
####
AC_OUTPUT

####
# print out a summary, what configure found out
####
echo
echo "Summary:"
echo "build system = $ac_sys_check_uname_o"
echo "AVR_GCC=${AVR_GCC}"
echo "AVR_LIBBFD_INC=${AVR_LIBBFD_INC}"
echo "AVR_LIBBFD_LIB=${AVR_LIBBFD_LIB}"
echo "AVR_LIBIBERTY_LIB=${AVR_LIBIBERTY_LIB}"
echo "PYTHON=${PYTHON}"
echo "TCL_WISH=${TCL_WISH}"
echo "build verilog modul avr.vpi = ${AVR_BUILD_VERILOG}"

# EOF

