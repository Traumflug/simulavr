#define PRG_WISH "/usr/bin/wish"
#define BFD_H "/home/zfrdh/work/binutils-2.19/build_avr/bfd/bfd.h"
#define VERSION "0.0000001 local version "

#ifdef __DO_NOT_USE_FROM_C_CODE

#if swig is available 
all: simulavr simulavr.so  #avr.vpi
#else  swig is not availabe
#all: simulavr 
#endif

#if ccache is available
CXX=ccache g++
#else
# CXX= g++
#endif

#some systems have no libtcl.so, they use libtcl8.4 instead so please set TCL_VERSION
#it is allowed to leave TCL_VERSION
TCL_VERSION=8.4 
#TCL_VERSION=

BFD= /home/zfrdh/work/binutils-2.19/build_avr

#HAVE_GEDA=yes
HAVE_GEDA=no
GEDA=/home/zfrdh/geda-install/
ADD_LIBS=/usr/lib/libz.a
#endif
