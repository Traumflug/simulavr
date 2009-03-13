#
#  $Id$
#

AC_DEFUN([AC_SWIG],
[
   AC_ARG_WITH(swig,[ --with-swig=PATH_TO_SWIG],
      [swig_path="$withval"]
      [swig_path=""])
   

   if test x"${swig_path}" = x; then
      AC_PATH_PROGS(swig_path, swig, swig,$PATH)
   fi

   AC_SUBST(swig_path)

])
