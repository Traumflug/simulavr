/*
 *   $Id$
 */

#include <stdio.h>
#include "debugio.h"
#include <avr/delay.h>
unsigned char getAdc(unsigned char);

void printIt(int n)
{
  printu0( "hello world #%d\n", n );
  _delay_ms( 100.0 );
  printk( "ADC0=%d expect %d\n", getAdc(0), n * 10 );
}

int main(
  int argc,
  char **argv
)
{
  /* for printk and printu0 (-R/-W IO and uart0 IO) */
  debugio_init();
  printIt( 1 );
  printIt( 2 );
  printIt( 3 );
  printIt( 1 );

  /* don't exit until the user forces to */
  while(1);
  return 0;
}
