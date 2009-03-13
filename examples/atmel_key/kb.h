/*
 *  $Id$
 */

// Keyboard communication routines

#ifndef __KB_INCLUDED
#define __KB_INCLUDED


#define ISC00 0
#define ISC01 1

void init_kb(void);
void decode(unsigned char sc);
void put_kbbuff(unsigned char c);
int getchar(void);
#endif

