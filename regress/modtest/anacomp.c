#include <avr/interrupt.h>

#include <avr/io.h>

void init(void) {
    // init port B: all port pins to output
    DDRB = 0xff;
    // set bandgap ref to AIN0
    ACSR = _BV(AINBG);
}

int main(void) {

    init();

    do {
        if((ACSR & _BV(ACO)) == _BV(ACO)) {
            PORTB = 0xff; // set output
        } else {
            PORTB = 0x00; // reset output
        }
    } while(1); // do forever
}

// EOF
