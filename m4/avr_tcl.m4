#
#  $Id$
#

AC_DEFUN([AX_TCL_ENVIRONMENT],
[
## Check if Tcl is desired
AC_MSG_CHECKING([Tcl desired])

# check for --enable-tcl
AC_ARG_ENABLE(tcl,
[AS_HELP_STRING([--enable-tcl],[enable Tcl use])],
[case "${enableval}" in
  yes) SIMULAVRXX_USE_TCL=yes ;;
  no) SIMULAVRXX_USE_TCL=no ;;
  *)  AC_MSG_ERROR(bad value ${enableval} for enable-tcl option) ;;
esac],[SIMULAVRXX_USE_TCL=no])

# check for --with-tclconfig
AC_ARG_WITH([tclconfig],
  [AS_HELP_STRING([--with-tclconfig=path  directory with tclConfig.sh])],
  [if test ! -d ${with_tclconfig} ; then
     AC_MSG_ERROR([(${with_tclconfig}) is not a directory])
   fi
   SIMULAVRXX_USE_TCL=yes
  ],
  [with_tclconfig=/usr/lib]
)

AC_MSG_RESULT([${SIMULAVRXX_USE_TCL}])

AC_MSG_RESULT([tclConfig.sh directory = $with_tclconfig])
# Check if Tcl development kit installed
if test x"${SIMULAVRXX_USE_TCL}" = x"yes" ; then
  # If we can find tclConfig.sh, forget hacking at it
  AC_CHECK_FILE(
    [${with_tclconfig}/tclConfig.sh],
    [source ${with_tclconfig}/tclConfig.sh
     Tcl_h_found=yes
     tclconfig_root_patch=${with_tclconfig}
     AC_SUBST([AVR_TCL_LIB],[${TCL_LIB_SPEC}])
     AC_SUBST([AVR_TCL_INCLUDE],[${TCL_INCLUDE_SPEC}])
     AC_SUBST([AVR_TCL_MODULE_SUFFIX],[${TCL_SHLIB_SUFFIX}])
    ],
    [
     AC_MSG_ERROR([tclConfig.sh not found])
    ]
  )
fi

HAVE_TCL_SHELLS=yes

## Some of the examples include GUIs written in Wish
if test ! x"${TCL_VERSION}" = x ; then
  AC_PATH_PROGS(TCL_WISH, wish${TCL_VERSION} wish${TCL_VERSION_MAJOR}${TCL_VERSION_MINOR} )
else
  TCL_WISH=no
fi
test "${TCL_WISH}" = no && AC_MSG_WARN([prefered version wish${TCL_VERSION} not found])
if test "${TCL_WISH}" = no ; then
  AC_PATH_PROGS(TCL_WISH, wish wish8.6 wish86 wish8.5 wish85 wish8.4 wish84 )
fi
test "${TCL_WISH}" = no && AC_MSG_WARN([wish not found])
AM_CONDITIONAL([HAVE_WISH], [test x$TCL_WISH != x])
AC_SUBST([TCL_WISH])
test x$TCL_WISH = x && HAVE_TCL_SHELLS=no

## Some of the examples include feedback modules written in Tclsh
if test ! x"${TCL_VERSION}" = x ; then
  AC_PATH_PROGS(TCL_SHELL, tclsh${TCL_VERSION} tclsh${TCL_VERSION_MAJOR}${TCL_VERSION_MINOR} )
else
  TCL_SHELL=no
fi
test "${TCL_SHELL}" = no && AC_MSG_WARN([prefered version tclsh${TCL_VERSION} not found])
if test "${TCL_SHELL}" = no ; then
  AC_PATH_PROGS(TCL_SHELL, tclsh tclsh8.6 tclsh86 tclsh8.5 tclsh85 tclsh8.4 tclsh84 )
fi
test "${TCL_SHELL}" = no && AC_MSG_WARN([tclsh not found])
AM_CONDITIONAL([HAVE_TCLSH], [test x$TCL_SHELL != x])
AC_SUBST([TCL_SHELL])
test x$TCL_SHELL = x && HAVE_TCL_SHELLS=no

## Test, if we have Itcl package available
check_itcl_available=no
if test ! x$TCL_SHELL = x ; then
  if (echo 'package require Itcl; exit 1;' | $TCL_SHELL); then
    AC_MSG_WARN([Itcl package not installed, tcl examples with gui will not work])
  else
    check_itcl_available=yes
  fi
fi

# If they did not want Tcl or it is not installed, do not use it
if test x"${SIMULAVRXX_USE_TCL}" = x"yes" -a x"${Tcl_h_found}" = x"yes"; then
  build_tcl_libs=yes
else
  build_tcl_libs=no
fi

AM_CONDITIONAL([USE_TCL], [test x"${build_tcl_libs}" = x"yes"])
AM_CONDITIONAL([TCL_ITCL_AVAILABLE], [test x"${check_itcl_available}" = x"yes"])
])
