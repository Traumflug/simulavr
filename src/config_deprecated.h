#
#  $Id$
#

#define PRG_WISH "/usr/bin/wish"

//#define WE_ACT_AS_SERVER 1

#ifdef __DO_NOT_USE_FROM_C_CODE

#if swig is available 
all: simulavr simulavr.so
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
#TCL_VERSION=8.4 
TCL_VERSION=


#endif
