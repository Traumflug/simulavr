/* show the reading & writing of eeprom memory space
 * also show working interrupts after eeprom write finished in
 * mega devices 
 *
 * Status: in work
 */

#include <avr/io.h>


volatile unsigned char compReady;

unsigned char results[0x20];


int main () {
    DDRB=0xff;
    volatile int x;

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




