AC_DEFUN([AC_PYTHON],
[
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
])
