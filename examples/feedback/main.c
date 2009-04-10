/*
 *   $Id$
 */

#include <stdio.h>
#include "debugio.h"

int main(
  int argc,
  char **argv
)
{
  /* for printk and printu0 (-R/-W IO and uart0 IO) */
  debugio_init();
  printu0( "hello world #1\n" );
  printu0( "hello world #2\n" );
  printu0( "hello world #3\n" );

  /* don't exit until the user forces to */
  while(1);
  return 0;
}
