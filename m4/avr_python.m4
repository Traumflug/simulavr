AC_DEFUN([AC_WAR_PYTHON],
[
 dnl This macro searches for python version 2.1.1 or newer
 AC_CACHE_CHECK([for python >= 2.1.1], [_cv_python_211], [

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
   have_python="yes"
 else
   AC_MSG_WARN([ ])
   AC_MSG_WARN([Python >= 2.1.1 not found.])
   AC_MSG_WARN([Regression tests will not be run.])
   AC_MSG_WARN([ ])
 fi
 AC_SUBST([ac_regression_subdir])
 AM_CONDITIONAL(COND_HAS_PYTHON, test "x$_cv_python_211" = "xyes")
])
