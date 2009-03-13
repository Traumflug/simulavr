/*
 *  This is a test program to demonstrate the program run limit feature.
 *
 *  $Id$
 */

#include "avrtest_help.h"

int main()
{
  volatile char in_char;

  debug_puts(
    "\n"
    "This program goes into an infinite loop after these messages.\n"
    "When run with the -m XXX option, the simulator should kill us.\n"
    "\n"
  );

  while (1) 
    ;       /* simulator should timeout */

  return 0;
}
