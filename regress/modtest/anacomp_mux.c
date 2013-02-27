/* this code is taken from examples/anacomp */
#include <avr/interrupt.h>

#include <avr/io.h>

volatile unsigned char in_loop = 0;

void init(void) {
    // init port B: only port pin 1 to output
    DDRB = 0x02;
    // AIN0 is positive input
    ACSR = 0;
    // ADC disable and set ACME bit in ADCSRB or SFIOR
    ADCSRA = 0;
#if defined(PROC_atmega16) || defined(PROC_atmega128) || defined(PROC_atmega8)
    SFIOR = _BV(ACME);
#else
    ADCSRB = _BV(ACME);
#endif
    // set AIN1 to ADC1 input
    ADMUX = 0x01;
}

int main(void) {

    init();

    do {
        in_loop = 1;
        if((ACSR & _BV(ACO)) == _BV(ACO)) {
            PORTB = 0x02; // set output
        } else {
            PORTB = 0x00; // reset output
        }
    } while(1); // do forever
}

// EOF
