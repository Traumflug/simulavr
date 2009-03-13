/* show the reading & writing of eeprom memory space
 * also show working interrupts after eeprom write finished in
 * mega devices 
 *
 * Status: in work
 *
 *  $Id$
 */

#include <avr/io.h>


volatile unsigned char compReady;

unsigned char results[0x20];


int main () {
   int i;
    DDRB=0xff;
    volatile int x;

    for( i=0; i<10000; ++i )
       *( (volatile char*) 0x20)='*';

    do {
        if (ACSR & (1<<ACO) ) {
            PORTB=0xff;
            x=1;
        } else {
           PORTB=0x00;
            x=0;
        }
    } while(1);

    return 0;
}




