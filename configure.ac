dnl Process this file with autoconf to produce a configure script.
AMDEP
AC_PREREQ(2.59)
AC_INIT
AC_CONFIG_SRCDIR([src/main.cpp])
AC_CONFIG_AUX_DIR(config)
AM_CONFIG_HEADER(src/config.h)
AM_INIT_AUTOMAKE(simulavrxx, 0.8.004)
AM_MAINTAINER_MODE


AC_PROG_CC
AC_PROG_CXX
AC_PROG_LIBTOOL

AC_CHECK_PROG(AVR_AS, avr-as, avr-as)
AC_CHECK_PROG(AVR_LD, avr-ld, avr-ld)
AC_CHECK_PROG(AVR_GCC, avr-gcc, avr-gcc)
AC_CHECK_PROG(AVR_GXX, avr-g++, avr-g++)
AC_PATH_PROG(TCL_WISH, wish wish8.4 wish8.3 wish8.2)
AC_CHECK_PROG(CCACHE, ccache, ccache, "")

if (echo $CXX | grep ccache); then
echo
else 
CC="$CCACHE $CC"
CXX="$CCACHE $CXX"
fi

AM_CONDITIONAL(HAVE_WISH, test x$TCL_WISH != x)

# find any old tcl, if present. (AC_SEARCH_LIBS more useful here? I
# think we don't want to always include it in LIBS which AC_SERCH_LIBS
# seems to do)
AC_CHECK_LIB(tcl,Tcl_Init,
[LIBTCL_NAME=tcl],
[
if test x"${TCL_WISH}" != "x"; then
        MY_WISH=`which ${TCL_WISH}`
        LIBTCL_NAME=`ldd ${MY_WISH} | grep libtcl | sed -e 's/[[\t ]]\+lib\(tcl[[^ \t]]\+\).so.*/\1/g'`
AS_MESSAGE(checking Checking for tcl via ldd wish: got ${LIBTCL_NAME}...)
else
AC_MSG_WARN([Checking for tcl via ldd wish: no wish found])
fi
])

AC_SUBST([AVR_AS])
AC_SUBST([AVR_LD])
AC_SUBST([AVR_GCC])
AC_SUBST([AVR_GXX])
AC_SUBST([TCL_WISH])
AC_SUBST([CXX])

dnl This macro searches for python version 2.1.1 or newer
AC_CACHE_CHECK(for python >= 2.1.1, _cv_python_211, [

changequote(<@, @>)

_cv_python_211='no' ;
if python -V 2>&1 | grep -q Python ; then
  cat <<EOF > py_ver.py
import sys
v = sys.version_info
ver = (int(v[0]) << 16) + (int(v[1]) << 8) + int(v[2])
if ver < 0x020101:
        print 'no'
else:
        print 'yes'
EOF
  _cv_python_211=`python py_ver.py`
  rm -f py_ver.py
fi

changequote([, ])
])

if test "x$_cv_python_211" = "xyes" ; then
  ac_regression_subdir="regress"
else
  AC_MSG_WARN([ ])
  AC_MSG_WARN([Python >= 2.1.1 not found.])
  AC_MSG_WARN([Regression tests will not be run.])
  AC_MSG_WARN([ ])
fi
AC_SUBST([ac_regression_subdir])
AM_CONDITIONAL(COND_HAS_PYTHON, test "x$_cv_python_211" = "xyes")



dnl Checks for programs.

dnl Checks for libraries.
AC_ARG_WITH( bfd-path,--with-bfd-path=path location of AVR-binutils version of libbfd,
[if test x"${withval}" != ""; then
   bfd_dir=${withval}
   libbfd_root_location=${bfd_dir}/${host}/avr
   AC_MSG_CHECKING(Any likely AVR-libbfd available via --with-bfd-path subtree...) 
   bfd_h_location=${libbfd_root_location}/include
   bfd_a_location=${libbfd_root_location}/lib
   if test -f ${bfd_h_location}/bfd.h -a \
      -f ${bfd_a_location}/libbfd.a; then
     echo "yes"
   else
     echo "no"
     bfd_h_location=""
   fi
  if test x"${bfd_h_location}" == x; then
   libbfd_root_location=${bfd_dir}
   AC_MSG_CHECKING(Any likely AVR-libbfd available via --with-bfd-path directory...) 
   bfd_h_location=${libbfd_root_location}/include
   bfd_a_location=${libbfd_root_location}/lib
   if test -f ${bfd_h_location}/bfd.h -a \
      -f ${bfd_a_location}/libbfd.a; then
     echo "yes"
   else
     echo "no"
     bfd_h_location=""
   fi
  fi
  if test x"${bfd_h_location}" == x; then
   libbfd_root_location=${bfd_dir}/avr
   AC_MSG_CHECKING(Any likely AVR-libbfd available via --with-bfd-path (2) directory...) 
   bfd_h_location=${libbfd_root_location}/include
   bfd_a_location=${libbfd_root_location}/lib
   if test -f ${bfd_h_location}/bfd.h -a \
      -f ${bfd_a_location}/libbfd.a; then
     echo "yes"
   else
     echo "no"
     bfd_h_location=""
   fi
  fi
  if test x"${bfd_h_location}" == x; then
   libbfd_root_location=${bfd_dir}
   AC_MSG_CHECKING(Any likely AVR-libbfd available via --with-bfd-path (3) directory...) 
   bfd_h_location=${libbfd_root_location}
   bfd_a_location=${libbfd_root_location}
   if test -f ${bfd_h_location}/bfd.h -a \
      -f ${bfd_a_location}/libbfd.a; then
     echo "yes"
   else
     echo "no"
     bfd_h_location=""
   fi
  fi
  if test x"${bfd_h_location}" == x; then
   libbfd_root_location=${bfd_dir}
   AC_MSG_CHECKING(Any likely AVR-libbfd available via --with-bfd-path (4) directory...) 
   bfd_h_location=${libbfd_root_location}/bfd
   bfd_a_location=${libbfd_root_location}/bfd
   if test -f ${bfd_h_location}/bfd.h -a \
      -f ${bfd_a_location}/libbfd.a; then
     echo "yes"
   else
     echo "no"
     bfd_h_location=""
   fi
  fi
fi
])

AVR_BFD_LIB
AVR_LIBIBLIO

dnl WAR:::hack in where wish is...should detect
wish_location=/usr/bin/wish

dnl some systems don't make "-ltcl" link to the version of tcl that
dnl the source code includes...so we try to detect it's installed name.


dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(fcntl.h)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST

dnl Checks for library functions.
AC_FUNC_VPRINTF

AC_SUBST([ac_aux_dir])
my_top_builddir=`pwd`/
AC_SUBST([my_top_builddir])
AC_SUBST([LIBTCL_NAME])
AC_SUBST([TCL_WISH])
AC_SUBST([CCACHE])

AC_CONFIG_FILES([Makefile \
        src/Makefile \
        regress/Makefile \
        regress/regress.py \
        regress/modules/Makefile \
        regress/test_opcodes/Makefile \
        examples/Makefile examples/anacomp/Makefile \
        examples/gui.tcl examples/anacomp/check.tcl \
        examples/anacomp/checkdebug.tcl ])
AC_OUTPUT

echo "LIBTCL_NAME        = " ${LIBTCL_NAME}
echo "libbfd.a location = " ${bfd_a_location}
echo "bfd.h location     = " ${bfd_h_location}
if test x"${LIBTCL_NAME}" == "x"; then
AC_MSG_WARN([
libtcl was not found. Problems may ensue. (if so, build proceedure should be updated)])
fi



