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

#if 1
  uart_1_puts( "what is up\n" );
  _delay_ms( 100.0 );
#endif

  /* for printk and printu0 (-R/-W IO and uart0 IO) */
  
  for ( i=1 ; i<13 ; i++ )
    printIt( i%8 );

  printk( "Sending exit request\n" );
  printu0( "E\n" );
  while(1) ; /* kill time until forced to exit */
  return 0;
}
