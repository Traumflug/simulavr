dnl I still don't understand hwo to be productive with GNU autotools...so 
dnl I'm sure I'm rolling my own when I shouldn't here...
dnl
dnl If you would like to work with me on doing things better here, pleae 
dnl let me know!


dnl AVR_CHECK_FOR_BFD( msg, root_path, relative_include_path, relative_lib_path )
dnl - defines AVR_LIBBFD_INCLUDE_DIR to detected include directory
dnl - defines AVR_LIBBFD_LIBDIR to detected library directory
dnl
AC_DEFUN([AVR_CHECK_FOR_BFD],
[ 
if test x"$bfd_h_location" == "x"; then
  AC_MSG_CHECKING($1 $2 in "$3" and "$4")
      bfd_h_location=$2/$3
      bfd_a_location=$2/$4
      if test -f ${bfd_h_location}/bfd.h -a \
         -f ${bfd_a_location}/libbfd.a; then
        AC_MSG_RESULT(yes)
        libbfd_root_location=$2
        echo set libbfd_root_location to ${libbfd_root_location}
      else
        AC_MSG_RESULT(no)
        bfd_a_location=""
        bfd_h_location=""
      fi
fi
])

dnl AVR_BFD_SEARCH_STEP(root_path)
AC_DEFUN([AVR_BFD_SEARCH_STEP],
[
   AVR_CHECK_FOR_BFD([bfd search-],[$1/${host}/avr],[include],[lib])
   AVR_CHECK_FOR_BFD([bfd search-],[$1/avr],[include],[lib])
   AVR_CHECK_FOR_BFD([bfd search-],[$1],[],[])
   AVR_CHECK_FOR_BFD([bfd search-],[$1],[bfd],[bfd])
   AVR_CHECK_FOR_BFD([bfd search-],[$1],[include],[lib])
])

AC_DEFUN([AVR_LIBIBERTY],
[
 if test x"$libiberty_location" == "x"; then
  if test x"$1" != x; then
   libiberty_location=$1
   AC_MSG_CHECKING(Any likely libiberty available via $1...) 
   if test -f ${libiberty_location}/libiberty.a; then
     AC_MSG_RESULT(yes)
   else
     AC_MSG_RESULT(no)
     libiberty_location=""
   fi
  fi 
 fi
AC_SUBST(libiberty_location)
])

if test x"${bfd_h_location}" == x; then
AC_MSG_ERROR([

*** ERROR ***

provide --with-bfd-path=path-to-your-avr-libbfd-files or
ensure your AVR cross-compipler toolset is in the path
\(read-elf is used to auto-detect where your 
AVR-build libbfd library is\)

for example, user the --enable-install-libbfd flag when configuring
binutils for AVR or execute make install_libbfd from your
AVR-binutil's bfd subdirectory to install the AVR-binutils version of
libbfd. Observe where the files are installed. For example:
--with-bfd-path=/home/some_user/install/i686-pc-linux-gnu/avr

This will make include/bfd.h and lib/libbfd.a available as required 
by this package

alternatively you may just keep your AVR-binutils build files around
and point to the bfd sibdirecotry there.

]
)
 fi
AC_SUBST(bfd_h_location)
AC_SUBST(bfd_a_location)
])
