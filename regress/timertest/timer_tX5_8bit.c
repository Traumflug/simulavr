/* timer test programm for timer1 on ATtinyX5: 8bit */
#include <avr/interrupt.h>

volatile int timer_ticks_overflow;
volatile int timer_ticks_compare_b;

void init_port(void) {
    /* set PB3 and PB4 to output, PB0-2 and PB5 as input with pullup (PB5 is allways input, if RSTDISBL
       is unprogrammed!) */
    DDRB = 0x18;
    /* set output port pins to 0, activate pullup on input pins. port B has only 6 bit! */
    PORTB = 0x27;
}
#ifdef TMODE_CTC
void init_timer1(void) {
    TCCR1 = 0x80; // CTC mode, no clock
    GTCCR = 0x10; // COM B: toggle on compare
    OCR1B = 0x40; // OCR = 64
    OCR1C = 0x7f; // OCRC = 127, e.g. overflow with 128µs
    PLLCSR = 0x00; // no PLL
    TIMSK |= _BV(TOIE1) + _BV(OCIE1B); // overflow interrupt (does not occur) + OCR1B interrupt
    TCNT1 = 0; // counter = 0
    TCCR1 = 0x81; // CTC mode, CKx1
}
#else
#ifdef TMODE_PWM
void init_timer1(void) {
    TCCR1 = 0x00; // no clock
    GTCCR = 0x50; // COM B: PWM, OC1B clear on compare and set on overflow
    OCR1B = 0x40; // OCR = 64, e.g. duty cycle 50%
    OCR1C = 0x7f; // OCRC = 127, e.g. overflow with 128µs
    PLLCSR = 0x02; // start PLL
    TIMSK |= _BV(TOIE1) + _BV(OCIE1B); // overflow interrupt (does not occur) + OCR1B interrupt
    TCNT1 = 0; // counter = 0
    DTPS1 = 0x03; // dead time generator prescaler: CKx8, e.g. 8MHz
    DT1B = 0x88; // dead time generator channel B: both edges are delayed with 1µs
}
#else
void init_timer1(void) {
    GTCCR = 0x10; // COM B: toggle on compare
    OCR1B = 0x40; // OCR = 64
    PLLCSR = 0x00; // no PLL
    TIMSK |= _BV(TOIE1) + _BV(OCIE1B); // overflow interrupt + OCR1B interrupt
    TCNT1 = 0; // counter = 0
    TCCR1 = 0x01; // CKx1
}
#endif
#endif

#ifdef TMODE_PWM
/* timer 1 interrupt: overflow */
ISR(TIMER1_OVF_vect) { timer_ticks_overflow++; }
#else
/* timer 1 interrupt: overflow, does not occur in CTC mode! */
ISR(TIMER1_OVF_vect) { timer_ticks_overflow++; GTCCR = 0x18; }
#endif

/* timer 1 interrupt: OCR channel B */
ISR(TIMER1_COMPB_vect) { timer_ticks_compare_b++; }

int main(void) {
    init_port();
    init_timer1();

    /* enable interrupts */
    sei();

#ifdef TMODE_PWM
    while((PLLCSR & 0x01) == 0) {} // wait for pll is locked
    PLLCSR = 0x06; // enable pll clock source for timer1, no LSM
    TCCR1 = 0x07; // start timer1, CKx64 e.g. 1µs period
#endif

    while(1) { } // stop processing here, go idle

    /* program flow dosn't come here */
    return 0;
}

/* EOF */
