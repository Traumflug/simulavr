dnl $Id$

AC_DEFUN([SIMULAVRXX_ENABLE_TCL],
[

AC_ARG_ENABLE(tcl,
[AS_HELP_STRING([--enable-tcl],[enable Tcl use])],
[case "${enableval}" in 
  yes) SIMULAVRXX_USE_TCL=yes ;;
  no) SIMULAVRXX_USE_TCL=no ;;
  *)  AC_MSG_ERROR(bad value ${enableval} for enable-tcl option) ;;
esac],[SIMULAVRXX_USE_TCL=yes]) 
])
