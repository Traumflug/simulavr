/* Example of the simulavrxx delivery.
 * This example shows the usage of the analog comparator
 * of the AVR mega devices the output of the ersult on Port B.
 * Compare AIN0 and ANI1. If AIN0 > AIN1 port B=0xff, else port B=0
 *
 * Status: in work
 *
 * $Id$
 */

#include <avr/io.h>

int main () {
   int i;
   volatile int x;     // For future use
   DDRB=0xff;          // Set all pins of port B to output
//   SFIOR |= (1<<ACME); // Enable Analog Multiplexer

#ifdef CONNECT_TO_WRITEPIPE
// To use this feature simulavrxx needs to be connect to
// a write pipe e.g. by the commandline switch -W0x20,-
   for( i=0; i<10000; ++i )
       *( (volatile char*) 0x20)='*';
#endif
//--------------------------------------------
   do {
       if (ACSR & (1<<ACO) ) { // Check the result
          PORTB=0xff;
          x=1;
        } else {
          PORTB=0x00;
          x=0;
        }
   } while(1);   // do forever
}




