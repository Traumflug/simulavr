/*
 *   $Id$
 */

#include <stdio.h>
#include "debugio.h"

void putstring(const char *c)
{
  const char *p = c;

  for ( ; *p ; p++ ) {
    fputc( *p, &uart0_str );
    fputc( *p, &debug_str );
  }
}

int main(
  int argc,
  char **argv
)
{
  debugio_init();
  putstring( "hello world" );
  /* printk( "hello world #1\n" );
  printk( "hello world #2\n" );
  printk( "hello world #3\n" ); */
  printu0( "JOEL\n" );
  while(1);
  return 0;
}
