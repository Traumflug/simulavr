##
## $Id$
##

####
# common initialisation of autotools
####
AC_INIT([SimulAVR], [1.1dev], [simulavr-devel@nongnu.org])
AM_INIT_AUTOMAKE([1.10 -Wall -Werror foreign subdir-objects])
AC_PREREQ([2.61])
AC_CONFIG_MACRO_DIR([m4])
AM_MAINTAINER_MODE

####
# check for libdl and libz, if it is necessary to link simulavr successful
####
AC_ARG_ENABLE(
  libdl,
  AC_HELP_STRING([--enable-libdl],
    [check an link libdl and libz, use only for distributions or OS, where it's necessary to link all together]),
    [
      link_libdl_enable="yes"
    ],[
      link_libdl_enable="no"
    ])
AC_CHECK_LIB(dl, dlopen, [EXTRA_LIBS_LDL="yes"])
AC_CHECK_LIB(z, deflate, [EXTRA_LIBS_LZ="yes"])
if test "$link_libdl_enable" = "yes"; then
  if test x"$EXTRA_LIBS_LDL" = x""; then
    AC_MSG_ERROR([required library libdl not found])
  fi
  if test x"$EXTRA_LIBS_LZ" = x""; then
    AC_MSG_ERROR([required library libz not found])
  fi
  EXTRA_LIBS="$EXTRA_LIBS -ldl -lz"
fi

####
# check for OS and build system: MSYS/MingW
####
AC_SYS_CHECK_MINGW
if test "$ac_sys_check_uname_o" = "MINGW32"; then
  AC_SUBST([PYTHON_MODULE_EXTENSION],[.pyd])
else
  AC_SUBST([PYTHON_MODULE_EXTENSION],[.so])
fi

####
# support of libtool, g++
####
AC_SUBST([DOLLAR_SIGN],[$])
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])
LT_INIT
LT_PREREQ([2.2])
AC_PROG_CXX
AC_CHECK_PROG(CXX_PROG,${CXX},yes,no)
if test ${CXX_PROG} = no ; then
  # AC_PROG_CXX sets g++, if nothing is found, but if not found make will fail
  AC_MSG_ERROR([C++ compiler ${CXX} not found],1)
fi

####
# support of swig (for tcl module and python module used)
####
echo "INFO: following warnings about SWIG not found can be ignored, if at least a valid"
echo "      installation with version > 1.3.18 is found!"
AC_PROG_SWIG(3.0.0)
if test "x$SWIG_LIB" = "x"; then
  # fall back to SWIG version 2.x.y in case of SWIG 3.x.y not found!
  AC_MSG_NOTICE([SWIG 3.x.y not available, try to find SWIG 2.x.y!])
  AC_PROG_SWIG(2.0.0)
  if test "x$SWIG_LIB" = "x"; then
    # fall back to SWIG version 1.3.x in case of SWIG 2.x.y not found!
    AC_MSG_NOTICE([SWIG 2.x.y or 3.x.y not available, try to find SWIG 1.3.x!])
    AC_PROG_SWIG(1.3.18)
  fi
fi
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
AZ_PYTHON_VERSION_ENSURE([2.6.0])
SWIG_PYTHON
AM_PATH_PYTHON
AM_CONDITIONAL([PYTHON_CMD_USE], [test ! -z "$PYTHON"])

###
# Regression test suite support programs
##
# If 'time' command is installed, we want to use it for the regression test suite,
# it's behavior this is different from the built-in 'time' command in bash.
AC_CHECK_PROG(GNUTIME,time,yes,no)
  
####
# Check about the AVR cross compilation environment, check for libz (only on msys,
# used automaticaly by libtool!)
####
AC_CHECK_PROG(AVR_AS, avr-as, avr-as)
AC_CHECK_PROG(AVR_LD,  avr-ld, avr-ld)
AC_CHECK_PROG(AVR_GCC, avr-gcc, avr-gcc)
AC_CHECK_PROG(AVR_GXX, avr-g++, avr-g++)
AC_SUBST([AVR_AS])
AC_SUBST([AVR_LD])
AC_SUBST([AVR_GCC])
AC_SUBST([AVR_GXX])
if test "$ac_sys_check_uname_o" = "MINGW32"; then
  # need libwsock32 too
  AVR_CHECK_WINSOCK
  # libtool links to libz by default, but on MSYS/MingW this is on other place
  AVR_CHECK_ZLIB_MSYS
  # add extra libs setting, if set on commandline, but only on msys
  AC_SUBST([EXTRA_LIBS])
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
AC_CHECK_HEADERS([vpi_user.h iverilog/vpi_user.h],
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
# support for some documentation tools, texinfo and doxygen
####
AC_CHECK_PROGS(TEX, [tex], [no])
AM_CONDITIONAL([HAVE_TEX], [test x"${TEX}" != x"no"])
AC_CHECK_PROG(MAKEINFO_PROG,makeinfo,yes,no)
if test ${MAKEINFO_PROG} = no ; then
  # makeinfo is used by creating info document
  AC_MSG_ERROR([makeinfo not found],1)
fi
DX_DOXYGEN_FEATURE(OFF)
DX_PDF_FEATURE(OFF)
DX_PS_FEATURE(OFF)
DX_INIT_DOXYGEN([Simulavr], [simulavr.doxy], )

####
# definitions for distcheck
####
## We have to propagate all the configure flags we want to use when
## configuring for make distcheck
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
  Makefile src/Makefile src/hwtimer/Makefile src/cmd/Makefile src/ui/Makefile
  src/python/Makefile src/python/setup.py doc/Makefile doc/conf.py doc/web/Makefile
  doc/web/conf.py doc/config.texi regress/Makefile regress/modules/Makefile
  regress/test_opcodes/Makefile regress/avrtest/Makefile regress/gtest/Makefile
  regress/timertest/Makefile regress/extinttest/Makefile regress/modtest/Makefile
  examples/verilog/Makefile examples/Makefile examples/anacomp/Makefile
  examples/atmega48/Makefile examples/atmega128_timer/Makefile
  examples/atmel_key/Makefile examples/feedback/Makefile examples/simple_ex1/Makefile
  examples/spi/Makefile examples/stdiodemo/Makefile examples/python/Makefile
  examples/simple_serial/Makefile
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
echo "PYTHON=${PYTHON}"
if test x"${build_tcl_libs}" = x"yes" -a x"${HAVE_TCL}" != x"no" ; then
  echo "TCL_WISH=${TCL_WISH}"
  echo "tcl has package Itcl? ${check_itcl_available}"
fi
echo "build verilog modul avr.vpi = ${AVR_BUILD_VERILOG}"
# EOF

