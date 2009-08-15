/*
 *  $Id$
 */

#include "avrtest_help.h"

/*
 *  This port correponds to the "-W 0x52,-" command line option.
 */
#define special_output_port (*( (volatile char *)0x52))

/*
 *  This port correponds to the "-e 0x4F" command line option.
 */
#define special_exit_port (*( (volatile char *)0x4F))

/*
 *  This port correponds to the "-a 0x49" command line option.
 */
#define special_abort_port (*( (volatile char *)0x49))

/*
 *  Poll the specified string out the debug port.
 */
void debug_puts(const char *str)
{
  const char *c;

  for ( c=str ; *c ; c++ )
    special_output_port = *c;
}

void sim_abort()
{
  special_abort_port = 0;
}

void sim_exit(int c)
{
  special_exit_port = (unsigned char)c;
}
