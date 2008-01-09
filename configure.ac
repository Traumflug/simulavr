AC_INIT([Simulavrxx], [0.8.007-WR_b], [simulavr-devel@nongnu.org])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_CONFIG_MACRO_DIR([m4])
AC_PROG_LIBTOOL
AC_PROG_CXX
AC_PROG_SWIG(1.3.17)
SWIG_ENABLE_CXX
SWIG_MULTI_MODULE_SUPPORT
SWIG_PYTHON
AM_CONDITIONAL([USE_SWIG],[test "x$SWIG" != 'x'])
AM_CONDITIONAL([USE_PYTHON],[test "x$PYTHON" != 'x'])


dnl AC_CHECK_PROG(CCACHE, ccache, ccache)


AX_AVR_ENVIRONMENT


AC_CONFIG_HEADERS([src/config.h])
AC_CONFIG_FILES([Makefile src/Makefile \
        examples/Makefile examples/anacomp/Makefile
        examples/gui.tcl
])
AC_OUTPUT

echo
echo AVR_LIBBFD_INC=${AVR_LIBBFD_INC}
echo AVR_LIBBFD_LIB=${AVR_LIBBFD_LIB}
echo AVR_LIBIBERTY_LIB=${AVR_LIBIBERTY_LIB}
echo PYTHON=${PYTHON}