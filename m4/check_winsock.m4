# AVR_CHECK_FOR_WINSOCK(msg, root_path, incl_ext, lib_ext)

AC_DEFUN([AVR_CHECK_FOR_WINSOCK], [ 
  if test x"$winsock_h_location" = "x"; then
    AC_MSG_CHECKING($1 $2 in "$3" and "$4")
    winsock_h_location=$2/$3
    winsock_a_location=$2/$4
    if test -f ${winsock_h_location}/winsock2.h -a \
            -f ${winsock_a_location}/libws2_32.a ; then
      AC_MSG_RESULT(yes)
        winsock_root_location="$2"
    else
      AC_MSG_RESULT(no)
      winsock_a_location=""
      winsock_h_location=""
    fi
  fi
])

# AVR_WINSOCK_SEARCH_STEP(root_path)

AC_DEFUN([AVR_WINSOCK_SEARCH_STEP], [
  AVR_CHECK_FOR_WINSOCK([winsock: search],[$1],[include],[lib])
])

# AVR_CHECK_WINSOCK()
# set C define HAVE_WINSOCK_H
# set make variable LIBWSOCK_FLAGS

AC_DEFUN([AVR_CHECK_WINSOCK], [
  # configure option to define the right path for winsock lib
  AC_ARG_WITH([winsock],
          [AS_HELP_STRING([--with-winsock=path  location of winsock lib on MSYS/MingW where include/winsock2.h and lib/libws2_32.a are found])],
          [],
          [with_winsock=check])
  AC_MSG_RESULT([with_winsock = $with_winsock])
  
  # seek for the lib and include file
  if test "${with_winsock}" != "check"; then
    AVR_WINSOCK_SEARCH_STEP($with_winsock)
  else
    AVR_WINSOCK_SEARCH_STEP(/mingw)
  fi
  
  # set variables for make and c code
  AC_DEFINE([HAVE_WINSOCK_H], [1], [Define to 1 if you build simulavr on windows with MSYS/MingW and winsock2.h was found])
  AC_SUBST([LIBWSOCK_FLAGS], ["-L${winsock_root_location}/lib -lws2_32"])
])

# EOF
