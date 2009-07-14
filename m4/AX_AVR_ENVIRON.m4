#
#  $Id$
#

dnl handle sanity checks for AVR environment and 
dnl special build requirements such as libbfd and
dnl libiberty built for the AVR toolchain.


dnl AX_AVR_ENVIRONMENT - check out AVR environment
AC_DEFUN([AX_AVR_ENVIRONMENT],
[AC_CHECK_PROG(AVR_AS, avr-as, avr-as)
AC_CHECK_PROG(AVR_LD,  avr-ld, avr-ld)
AC_CHECK_PROG(AVR_GCC, avr-gcc, avr-gcc)
AC_CHECK_PROG(AVR_GXX, avr-g++, avr-g++)
AC_CHECK_PROG(AVR_NM,  avr-nm,  avr-nm)
AC_CHECK_PROG(NATIVE_NM, nm, nm)


dnl Let's handle user-provided flags first.
AC_ARG_WITH([bfd],
        [AS_HELP_STRING([--with-bfd=path  location of AVR-binutils version of libbfd install where include/bfd.h and lib/libbfd.a are found (from binutils)])],
        [],
        [with_bfd=check])
AC_MSG_RESULT([with_bfd = $with_bfd])

AC_ARG_WITH([libiberty],
        [AS_HELP_STRING([--with-libiberty=path location of libiberty(from binutils)])],
        [],
        [with_libiberty=check])
AC_MSG_RESULT([with_libiberty = $with_libiberty])

dnl need to use given library if it seems to be an avr-build
dnl libbfd. If given library is not suitable, error out.
dnl if no libbfd is given, and with_bfd is not "no", then search
dnl for libbfd. If none found, error out.

if test x"${with_bfd}" != "xcheck"; then
  AVR_BFD_SEARCH_STEP($with_bfd)
else
  AVR_BFD_SEARCH_STEP(/usr)
fi

if test -z "${bfd_a_location}"; then
  AC_MSG_ERROR([
    Could not locate libbfd.so/libbfd.a and/or bfd.h.
    Please use the --with-bfd=<path to your libbfd library>
 ])
fi

AVR_LIBBFD_LIBPATH=${bfd_a_location}
if test -f "${bfd_a_location}/libbfd.so"; then
  AVR_LIBBFD_LIB=${bfd_a_location}/libbfd.so
else
  AVR_LIBBFD_LIB=${bfd_a_location}/libbfd.a
fi
AVR_LIBBFD_INC=${bfd_h_location}

######### LIBIBERTY
if test "x${with_libiberty}" != "xcheck"; then
  AVR_LIBIBERTY_SEARCH_STEP($with_libiberty)
else
  if test x"${with_bfd}" != "xcheck"; then
    AVR_LIBIBERTY_SEARCH_STEP($with_bfd)
  fi
  AVR_LIBIBERTY_SEARCH_STEP(/usr)
fi

if test -z "${libiberty_a_location}";
then
  AC_MSG_ERROR([
    Could not locate libiberty.so/libiberty.a
    Please use the --with-libiberty=<path to your libiberty library>
 ])
fi

AVR_LIBIBERTY_LIBPATH=${libiberty_a_location}
if test -f "${libiberty_a_location}/libiberty.so"; then
  AVR_LIBIBERTY_LIB=${libiberty_a_location}/libiberty.so
else
  AVR_LIBIBERTY_LIB=${libiberty_a_location}/libiberty.a
fi

## BEGIN CYGWIN HACKS
## It appears that this is not needed on Fedora 10 but is needed on Cygwin
## TBD: Better way to figure out when this is really needed
if test -r /usr/lib/libcygwin.a ; then 
  AVR_LIBBFD_LIB="/usr/lib/libcygwin.a ${AVR_LIBBFD_LIB}"
fi
if test -r ${libiberty_a_location}/libintl.a ; then 
  AVR_LIBIBERTY_LIB="${AVR_LIBIBERTY_LIB} ${libiberty_a_location}/libintl.a"
fi
if test -r ${libiberty_a_location}/libiconv.a ; then 
  AVR_LIBIBERTY_LIB="${AVR_LIBIBERTY_LIB} ${libiberty_a_location}/libiconv.a"
fi
## END OF CYGWIN HACKS


## Now substitute with all that we found
AC_SUBST([AVR_AS])
AC_SUBST([AVR_LD])
AC_SUBST([AVR_GCC])
AC_SUBST([AVR_GXX])
AC_SUBST([AVR_NM])
AC_SUBST([NATIVE_NM])
AC_SUBST([AVR_LIBBFD_LIB])
AC_SUBST([AVR_LIBBFD_LIBPATH])
AC_SUBST([AVR_LIBBFD_INC])
AC_SUBST([AVR_LIBIBERTY_LIB])
AC_SUBST([AVR_LIBIBERTY_LIBPATH])
])
