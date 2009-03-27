/*
 *  $Id$
 */

#ifndef __DEBUGIO_h
#define __DEBUGIO_h

#include <stdio.h>


extern FILE debug_str;
extern FILE uart0_str;

#define printk(...) fprintf( &debug_str, __VA_ARGS__ )
#define printu0(...) fprintf( &uart0_str, __VA_ARGS__ )

void debugio_init(void);

#endif
