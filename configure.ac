dnl Process this file with autoconf to produce a configure script.
AMDEP
AC_PREREQ(2.59)
AC_INIT([Simulavrxx], 0.8.006, [Klaus Rudolph <lts-rudolph@gmx.de>], [simulavrxx])
AC_CONFIG_SRCDIR([src/main.cpp])
AM_CONFIG_HEADER(src/config.h)
AC_CONFIG_AUX_DIR(config)

AM_INIT_AUTOMAKE
AM_MAINTAINER_MODE

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_LIBTOOL
AC_SWIG
AC_WAR_TCL

AC_CHECK_PROG(AVR_AS, avr-as, avr-as)
AC_CHECK_PROG(AVR_LD, avr-ld, avr-ld)
AC_CHECK_PROG(AVR_GCC, avr-gcc, avr-gcc)
AC_CHECK_PROG(AVR_GXX, avr-g++, avr-g++)
AC_CHECK_PROG(CCACHE, ccache, ccache, "")

AC_SUBST([AVR_AS])
AC_SUBST([AVR_LD])
AC_SUBST([AVR_GCC])
AC_SUBST([AVR_GXX])
AC_SUBST([CCACHE])


# Checks for libraries.
AC_CHECK_LIB([ncurses], [cbreak])

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([arpa/inet.h fcntl.h netinet/in.h stdlib.h string.h sys/socket.h unistd.h])


# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_TYPE_SIZE_T
AC_C_VOLATILE
AC_C_BIGENDIAN

if test -e ${swig_path}; then
 AC_MSG_NOTICE([got swig user parameter])
else
 swig_path=""
fi
AM_CONDITIONAL([USE_SWIG],[test x${swig_path} != "x"])

if (echo $CXX | grep ccache); then
  echo
else 
  CC="$CCACHE $CC"
  CXX="$CCACHE $CXX"
fi

AC_SUBST([CXX])


# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_TYPE_SIGNAL
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([inet_ntoa memset socket strdup strerror])

AC_WAR_PYTHON

if test "x$_cv_python_211" = "xyes" ; then
  ac_regression_subdir="regress"
else
  AC_MSG_WARN([ ])
  AC_MSG_WARN([Python >= 2.1.1 not found.])
  AC_MSG_WARN([Regression tests will not be run.])
  AC_MSG_WARN([ ])
fi
AC_SUBST([ac_regression_subdir])
AM_CONDITIONAL(COND_HAS_PYTHON, [test "x$_cv_python_211" = "xyes"])


AC_ARG_WITH([bfd-path],
	    [--with-bfd-path=path location of AVR-binutils version of libbfd],
	    [AVR_BFD_SEARCH_STEP([${withval}])
])
if test x"$bfd_h_location" = "x"; then
  libbfd_root_location=$(which avr-readelf | sed -e ['s/\/[^\/]*\/[^\/]*$//']) 2> /dev/null

   # Gentoo puts the AVR version of tools somewhere like this after 
   # a "crossdev -t avr" install of the compiler toolchain
   my_binutils_ver=$(avr-readelf  --version | head -n 1 | sed -e 's/GNU readelf //' )
  AVR_BFD_SEARCH_STEP([/usr/binutils/avr/${my_binutils_ver}])  
  AVR_BFD_SEARCH_STEP([${libbfd_root_location}  /${my_binutils_ver}])  
  AVR_BFD_SEARCH_STEP([${libbfd_root_location}/${host}/avr])
  AVR_BFD_SEARCH_STEP([${libbfd_root_location}])
fi

AC_ARG_WITH([libiberty-path],
	    [--with-libiberty-path=path location of AVR-binutils version of libiberty],
	    [AC_MSG_NOTICE([--with-libiberty-path=${withval} specified...])
             AVR_LIBIBERTY([${withval}])
])

if test x"${bfd_h_location}" = x; then
AC_MSG_ERROR([

*** ERROR ***

Provide --with-bfd-path=path-to-your-avr-bfd.h to ".\configure" or ensure
your AVR cross-compiler toolset is in the path (read-elf is used to auto-detect
where your AVR-build libbfd library is).

For example, use the --enable-install-libbfd flag when configuring binutils for
the AVR or execute "make install_libbfd" from your AVR-binutils' bfd subdirectory.
Observe where the files are installed. For example:

  ./configure --with-bfd-path=/home/someuser/build-avr-binutils/bfd

This will make include/bfd.h and lib/libbfd.a available as required 
by this package.

Alternatively, you may just keep your AVR-binutils build files around
and point to the bfd subdirectory there.

GNU locate may be useful: locate bfd.h
and see if there's a bfd.h in any of
the AVR directories.

])
fi

AC_SUBST([bfd_h_location])
AC_SUBST([bfd_a_location])

echo set libbfd_root_location to ${libbfd_root_location}

AVR_LIBIBERTY([${libbfd_root_location}])
AVR_LIBIBERTY([${libbfd_root_location}/libiberty])
avr_readelf=$(dirname $(dirname [$(which avr-readelf])))
AVR_LIBIBERTY([${avr_readelf}/lib])
if test x${with_bfd_path} != x; then
AVR_LIBIBERTY([${with_bfd_path}/lib])
fi

if test x"${libiberty_location}" = x; then
AC_MSG_ERROR([
 
*** ERROR ***

libiberty.a from binutils is not found
])
fi

AC_SUBST([ac_aux_dir])
my_top_builddir=`pwd`/
AC_SUBST([my_top_builddir])

# Attempt to provide preliminary support for OSX (darwin?) 
# I don't know enough about this, so I've simply hacked a darwin check here to 
# cue modifications to the Makefiles later....
# If you know a better way, please contact this package's maintainers.
case $host_os in
   darwin*)
      NonPortableDarwin_LIBADD_Hack='$(bfd_a_location)/libbfd.a $(libiberty_location)/libiberty.a'
      NonPortableDarwin_GXX_Flags='-flat_namespace -single_module -dynamiclib -undefined suppress'
      ;;
        *)
      NonPortableDarwin_LIBADD_Hack=
      NonPortableDarwin_GXX_Flags=
      ;;
esac
AC_SUBST([NonPortableDarwin_GXX_Flags])
AC_SUBST([NonPortableDarwin_LIBADD_Hack])

if test "x${swig_path}" = "x"; then
AC_MSG_WARN([
*** WARNING ***
swig not found, so it is not possible to build simulavr.so and tcl examples will not run. Though simulavrxx itself may be built without it.
])
fi

AM_CONDITIONAL([WAR_SHARED],[test x$enable_shared = "xyes"])

if test x"$enable_shared" = "xyes"; then
 if test x"$swig_path" != "x"; then
  if test x"$TCL_WISH" != "x"; then
     use_wish=yes
  fi
  if test x"$have_python" != "x"; then
     use_python=yes
  fi
 fi
fi
AM_CONDITIONAL([USE_PYTHON],[test "x$have_python" != "x"])
AM_CONDITIONAL([USE_WISH],[test "x$TCL_WISH" != "x"])

AC_CONFIG_FILES([Makefile
        doc/Makefile
        src/Makefile
        src/python/Makefile
        regress/Makefile
        regress/regress.py
        regress/modules/Makefile
        regress/test_opcodes/Makefile
        examples/Makefile examples/anacomp/Makefile examples/simple_ex1/Makefile
        examples/gui.tcl examples/anacomp/check.tcl
        examples/anacomp/checkdebug.tcl
        examples/anacomp/checkdebug.gdb
         ])
AC_OUTPUT


echo "swig_path         = " ${swig_path}
echo "libbfd.a location = " ${bfd_a_location}
echo "bfd.h location     = " ${bfd_h_location}
echo "libiberty location = " ${libiberty_location}
echo "tcl include        = " ${TCL_INCLUDE_SPEC}
echo "tcl version        = " ${TCL_VERSION}
echo "tcl lib spec       = " ${TCL_LIB_SPEC}
echo "my tcl lib spec    = " ${MYTCL_LIB_SPEC}
echo "swig_path          = " ${swig_path}
