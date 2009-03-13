/*
 *  This is a test program to demonstrate the simulator abort magic address.
 *
 *  $Id$
 */

#include "avrtest_help.h"

int main()
{
  debug_puts(
    "\n"
    "This program tests the simulator magic abort port.\n"
    "There should be no more messages after this one.\n"
  );

  sim_abort();

  debug_puts( "ERROR - Simulator did not abort?\n" );

  return 0;
}
