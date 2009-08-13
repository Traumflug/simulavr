# AVR_CHECK_FOR_ZLIB(msg, root_path, lib_ext)

AC_DEFUN([AVR_CHECK_FOR_ZLIB], [ 
  if test x"$zlib_a_location" = "x"; then
    AC_MSG_CHECKING($1 $2 in "$3")
    zlib_a_location=$2/$3
    if test -f ${zlib_a_location}/libz.a ; then
      AC_MSG_RESULT(yes)
        zlib_root_location="$2"
    else
      AC_MSG_RESULT(no)
      zlib_a_location=""
    fi
  fi
])

# AVR_ZLIB_SEARCH_STEP(root_path)

AC_DEFUN([AVR_ZLIB_SEARCH_STEP], [
  AVR_CHECK_FOR_ZLIB([zlib: search],[$1],[lib])
])

# AVR_CHECK_ZLIB_MSYS()
# set make variable LIBZ_FLAGS

AC_DEFUN([AVR_CHECK_ZLIB_MSYS], [
  # configure option to define the right path for zlib
  AC_ARG_WITH([zlib],
          [AS_HELP_STRING([--with-zlib=path  location of zlib on MSYS/MingW where lib/libz.a is found])],
          [],
          [with_zlib=check])
  AC_MSG_RESULT([with_zlib = $with_zlib])
  
  # seek for the lib
  if test "${with_zlib}" != "check"; then
    AVR_ZLIB_SEARCH_STEP($with_zlib)
  else
    AVR_ZLIB_SEARCH_STEP(/usr)
  fi
  
  # set variables for make
  AC_SUBST([LIBZ_FLAGS], ["-L${zlib_root_location}/lib -lz"])
])

# EOF
