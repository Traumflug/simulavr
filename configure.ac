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

## Check if Tcl is desired
AC_MSG_CHECKING([Tcl desired])
SIMULAVRXX_ENABLE_TCL
AC_MSG_RESULT([${SIMULAVRXX_USE_TCL}])

AC_ARG_WITH([tclconfig],
  [AS_HELP_STRING([--with-tclconfig=path  directory with tclConfig.sh])],
  [if test ! -d ${with_tclconfig_path} ; then
     AC_MSG_ERROR([(${with_tclconfig_path}) is not a directory])
   fi
  ],
  [with_tclconfig_path=/usr/lib]
)

AC_MSG_RESULT([tclConfig.sh directory = $with_tclconfig_path])
# Check if Tcl development kit installed
if test x"${SIMULAVRXX_USE_TCL}" = x"yes" ; then
  # If we can find tclConfig.sh, forget hacking at it
  AC_CHECK_FILE(
    [${with_tclconfig_path}/tclConfig.sh],
    [source ${with_tclconfig_path}/tclConfig.sh
     Tcl_h_found=yes
     tclconfig_root_patch=${with_tclconfig_path}
     AC_SUBST([AVR_TCL_LIB],[${TCL_LIB_SPEC}])
     AC_SUBST([AVR_TCL_INCLUDE],[${TCL_INCLUDE_SPEC}])
    ],
    [AC_MSG_WARN([tclConfig.sh not found .. guessing])
     AC_CHECK_HEADER([tcl.h], [Tcl_h_found=yes], [Tcl_h_found=no])
     AC_CHECK_HEADER([tcl.h], [Tcl_h_found=yes], [Tcl_h_found=no])
     AC_SUBST([AVR_TCL_LIB],[-ltcl8.5])
     AC_SUBST([AVR_TCL_INCLUDE],[])
    ]
  )
fi

## Some of the examples include GUIs written in Wish
AC_PATH_PROG(TCL_WISH, [wish], [/usr/bin/wish], [no])
test "${TCL_WISH}" = no && AC_MSG_WARN([wish not found])

# If they did not want Tcl or it is not installed, do not use it
if test x"${SIMULAVRXX_USE_TCL}" = x"yes" -a x"${Tcl_h_found}" = x"yes"; then
  build_tcl_libs=yes
else
  build_tcl_libs=no
fi

AM_CONDITIONAL([USE_TCL], [test x"${build_tcl_libs}" = x"yes"])

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
if test x"${build_tcl_libs}" = x"yes" -a x"${TCL_WISH}" != x"no" ; then
  AC_CONFIG_FILES(
    [examples/gui.tcl],
    [chmod +x examples/gui.tcl])
  AC_CONFIG_FILES(
    [examples/simulavr.tcl],
    [chmod +x examples/simulavr.tcl])
  AC_CONFIG_FILES(
    [examples/stdiodemo/checkdebug.tcl],
    [chmod +x examples/stdiodemo/checkdebug.tcl])
  AC_CONFIG_FILES(
    [examples/spi/check.tcl],
    [chmod +x examples/spi/check.tcl])
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
  examples/Makefile examples/anacomp/Makefile examples/atmel_key/Makefile \
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

