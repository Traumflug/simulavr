AC_DEFUN([AVR_LIBIBLIO],
[
  libbfd_root_location=$(which avr-readelf | sed -e ['s/\/[^\/]*\/[^\/]*$//']) 2> /dev/null
  if test x"${libbfd_root_location=$}" != x; then
   libiberty_location="${libbfd_root_location}/lib"
   AC_MSG_CHECKING(Any likely libiberty available via system path...) 
   if test -f ${libiberty_location}/libiberty.a; then
     echo "yes"
   else
     echo "no"
     libiberty_location=""
   fi
  fi 
AC_SUBST(libiberty_location)
])

AC_DEFUN([AVR_BFD_LIB],
[
 if test x"${bfd_h_location}" == x; then
  if test x"${prefix}" != xNONE; then
   libbfd_root_location=${prefix}/${host}/avr
   AC_MSG_CHECKING(Any likely AVR-libbfd available via --prefix...) 
   bfd_h_location=${libbfd_root_location}/include
   bfd_la_location=${libbfd_root_location}/lib
   if test -f ${bfd_h_location}/bfd.h -o \
      -f ${bfd_la_location}/libbfd.la; then
     echo "yes"
   else
     echo "no"
     bfd_h_location=""
   fi
  fi
 fi
 if test x"${bfd_h_location}" == x; then
  libbfd_root_location=$(which avr-readelf | sed -e ['s/\/[^\/]*\/[^\/]*$//']) 2> /dev/null
  if test x"${libbfd_root_location}" != x; then
   libbfd_root_location="${libbfd_root_location}/${host}/avr"
   AC_MSG_CHECKING(any likely AVR-libbfd available via avr-binutils install path..) 
   bfd_h_location=${libbfd_root_location}/include
   bfd_la_location=${libbfd_root_location}/lib
   if test -f ${bfd_h_location}/include/bfd.h -o \
      -f ${bfd_la_location}/libbfd.la; then
     echo "yes"
   else
     echo "no"
     bfd_h_location=""
   fi
  fi 
 fi

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
AC_SUBST(bfd_la_location)
])
