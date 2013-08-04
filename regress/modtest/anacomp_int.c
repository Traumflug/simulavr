/* this code is taken from examples/anacomp */
#include <avr/interrupt.h>

#include <avr/io.h>

volatile unsigned char isr_count = 0;
volatile unsigned char in_loop = 0;

void init(void) {
    // init port B: only port pin 0 to output
    DDRB = 0x01;
#ifdef PROC_at90s4433
    // set bandgap ref to AIN0
    ACSR = _BV(AINBG) | _BV(ACIE);
#else
#ifdef PROC_at90s8515
    // AIN0 from external source
    ACSR = 0x00;
#else
    // set bandgap ref to AIN0
    ACSR = _BV(ACBG);
#endif
#endif
}

void set_port(void) {
    if((ACSR & _BV(ACO)) == _BV(ACO)) {
        PORTB = 0x01; // set output
    } else {
        PORTB = 0x00; // reset output
    }
}

#if defined(PROC_attiny25) || defined(PROC_atmega8) || defined(PROC_at90s4433) || defined(PROC_at90s8515) || defined(PROC_attiny2313) || defined(PROC_atmega16)
ISR(ANA_COMP_vect) {
#else
ISR(ANALOG_COMP_vect) {
#endif
    // set port according to comparator output
    set_port();
    // count isr counter
    isr_count++;
}

int main(void) {

    init();

    // set port to initial state depending on ACO
    set_port();

    // reset ACIF to not to trigger interrupt immediately after sei
#ifdef PROC_at90s4433
    ACSR = _BV(AINBG) | _BV(ACIE) | _BV(ACI);
#else
#ifdef PROC_at90s8515
    ACSR = _BV(ACIE) | _BV(ACI);
#else
    ACSR = _BV(ACBG) | _BV(ACIE) | _BV(ACI);
#endif
#endif
    
    // enable interrupts
    sei();

    do {
        in_loop = 1;
    } while(1); // do forever
}

// EOF
