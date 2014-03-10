# AC_SYS_CHECK_MINGW( )
# -----------------
# Checks, if build system is MSYS/MingW or not.
# Sets SYS_MINGW for makefile usage and HAVE_SYS_MINGW for usage in C code
# Further sets shell variable ac_sys_check_uname_o with result of "uname -o"

AC_DEFUN([AC_SYS_CHECK_MINGW], [
  AC_MSG_CHECKING(whether build system is MSYS/MingW)
  ac_sys_check_uname_o=`uname -s | cut -b-7`
  if test "$ac_sys_check_uname_o" = "MINGW32"; then
    AM_CONDITIONAL([SYS_MINGW], [true])
    AC_DEFINE([HAVE_SYS_MINGW], [1], [Define to 1 if you build simulavr on windows with MSYS/MingW])
    AC_MSG_RESULT(yes)
  else
    AC_MSG_RESULT(no)
    AM_CONDITIONAL([SYS_MINGW], [false])
  fi
])

# EOF
