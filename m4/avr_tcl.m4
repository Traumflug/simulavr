#
#  $Id$
#

AC_DEFUN([AC_WAR_TCL],
[
   AC_PATH_PROG(TCL_WISH, wish wish8.4 wish8.3 wish8.2 )
   AC_SUBST([TCL_WISH])

   AM_CONDITIONAL([HAVE_WISH], [test x$TCL_WISH != x])

   # look for tcl config file...it should give us good information. again ignores user parameter ;-(
   AC_PATH_PROGS(TCL_CONFIG, [tclConfig.sh], [""], [/usr/lib/tcl /usr/lib/tcl8.4 /usr/lib/tcl8.3 /usr/lib/tcl8.2 /usr/lib])

   # find any old tcl, if present. (AC_SEARCH_LIBS more useful here? I
   # think we don't want to always include it in LIBS which AC_SERCH_LIBS
   # seems to do)
   if test x"${TCL_CONFIG}" != "x"; then
       source ${TCL_CONFIG}
       # WAR: I don't know what I'm doing...TCL_LIB_SPEC contains
       # an unepanded variable ${TCL_DBGX} when I looked..
       MYTCL_LIB_SPEC=`eval echo "${TCL_LIB_SPEC}"`
       echo "MYTCL_LIB_SPEC=$MYTCL_LIB_SPEC"
   else
       AC_MSG_WARN([Failed to find tclConfig.sh... tcl dependent code may fail to build...ask package maintainer to update configure process])
   fi
   AM_CONDITIONAL([HAVE_TCL_DEV], [test x$TCL_INCLUDE_SPEC != x])
   
   
   AC_SUBST([TCL_INCLUDE_SPEC])
   AC_SUBST([TCL_LIB_SPEC])
   AC_SUBST([MYTCL_LIB_SPEC])
])
