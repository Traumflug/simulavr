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
AC_ARG_WITH([bfd-path],
        [AS_HELP_STRING([--with-bfd=path  location of AVR-binutils version of libbfd install where include/bfd.h and lib/libbfd.a are found (from binutils)])],
        [],
        [with_bfd_path=check])
AC_MSG_RESULT([with_bfd_path = $with_bfd_path])

AC_ARG_WITH([libiberty-path],
        [AS_HELP_STRING([--with-libiberty=path  location of libiberty(from binutils)])],
        [],
        [with_libiberty_path=check])
AC_MSG_RESULT([with_libiberty_path = $with_libiberty_path])

dnl need to use given library if it seems to be an avr-build
dnl libbfd. If given library is not suitable, error out.
dnl if no libbfd is given, and with_bfd_path is not "no", then search
dnl for libbfd. If none found, error out.

if test x"${with_bfd_path}" = "xcheck";
then
   AX_TEST_LIBBFD([$(dirname $(which ${AVR_AS}))/../$host/avr/])
fi


if test x"${with_bfd_path}" != "xcheck";
then
  AX_TEST_LIBBFD($with_bfd_path)
fi

if test -z "${AVR_LIBBFD_LIB}";
then
  AC_MSG_ERROR([${with_bfd_path} DOES NOT appear to be the location of the avr-built lib/libbfd.a or include/bfd.h or is an incompatable version of the avr libbfd\n Please use the --with-bfd-path=<path to your avr-built libbfd library>])
fi

if test "x${with_libiberty_path}" = "xcheck";
then
   AX_TEST_LIBIBERTY([$(dirname $(which ${AVR_AS}))/../lib/libiberty.a])
fi

AC_SUBST([AVR_AS])
AC_SUBST([AVR_LD])
AC_SUBST([AVR_GCC])
AC_SUBST([AVR_GXX])
AC_SUBST([AVR_NM])
AC_SUBST([NATIVE_NM])
AC_SUBST([CCACHE])
AC_SUBST([AVR_LIBBFD_LIB])
AC_SUBST([AVR_LIBBFD_INC])
AC_SUBST([AVR_LIBIBERTY_LIB])
])

dnl AX_CHECK_ACCEPT_FILE_VALUE( FILE, VAR_NAME )
dnl Check that FILE exists or AC_MSG_ERROR out
dnl If file exists, set VAR_NAME with value FILE

dnl AX_TEST_LIBBFD( LIBBFD_PATH )
AC_DEFUN([AX_TEST_LIBBFD],
[AS_IF([test -d $1],
  [
  AS_IF([(${NATIVE_NM} $1/lib/libbfd.a  | grep -q -e '^elf32-avr.o:$') &&
         (${NATIVE_NM} $1/lib/libbfd.a  | grep -q -e '^[[0-9a-f]]\+ T [[_]]\?bfd_check_format$') ],
        [AC_MSG_RESULT([ libbfd.a library looks ok])
        [AVR_LIBBFD_LIB]=[$1/lib/libbfd.a]]
        [AVR_LIBBFD_INC]=[$1]/include)

  ],
  [AC_MSG_RESULT([$1 is NOT a directory...])])])

dnl AX_TEST_LIBIBERTY( LIBIBERTY_PATH )
AC_DEFUN([AX_TEST_LIBIBERTY],
[AS_IF([test -f $1],
        AC_MSG_RESULT([found file $1])
        [AVR_LIBIBERTY_LIB]=[$1],
        AC_MSG_RESULT([$1 is not a file]))]
)
