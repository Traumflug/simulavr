/*
 *   $Id$
 */

#include <stdio.h>
#include "debugio.h"
#include <util/delay.h>
unsigned char getAdc(unsigned char);

void printIt(int n)
{
  int i;

  printu0( "hello world #%d\n", n );
  _delay_ms( 100.0 );
  for ( i=0 ; i<8 ; i++ ) 
    printk( "ADC%d=%3d ", i, getAdc(i) );
  printk( "\n" );
}

int main(
  int argc,
  char **argv
)
{
  int i;

  debugio_init();

  /* for printk and printu0 (-R/-W IO and uart0 IO) */
  
  for ( i=1 ; i<13 ; i++ )
    printIt( i%8 );

  /* don't exit until the user forces to */
  while(1);
  return 0;
}
