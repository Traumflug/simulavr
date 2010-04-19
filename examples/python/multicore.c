/*
 * Example code for multicore example with python interface
 */
 
#include <avr/interrupt.h>

#ifdef DUAL_B

// Code for core B: count events on port B2

volatile char cnt_irq = 0;         // IRQ counter
volatile char cnt_res = 0;         // measurement value for time distance between events

ISR(SIG_INTERRUPT0) {
  cnt_irq++;                       // increment event counter
  cnt_res = TCNT0;                 // save timer value = time distance to last event
  TCNT0 = 0;                       // reset timer value
}

void init() {
  MCUCR = _BV(ISC00) | _BV(ISC01); // rising edge will generate interrupt
  GICR |= _BV(INT0);               // enable INT0
  GIFR |= _BV(INTF0);              // reset former irq request
  TCNT0 = 0;                       // Timer 0 as counter with internal clock
  TCCR0 = 0x0 + 0x04 + 0x0;        // wgm=0, cs=4, com=0
  sei();                           // enable interrupts
}

#endif

#ifdef DUAL_A

// Code for core A: generate 250Hz signal on port B3

void init() {
  TCNT0 = 0;                       // reset timer
  OCR0 = 124;                      // Timer 0 by CLK/64, 2ms on 4MHz, CTC mode,
                                   // 4ms period on OC0 out
  TCCR0 = 0x08 + 0x03 + 0x10;      // wgm=2, cs=3, com=1
  DDRB = 0x8;                      // PB3=out
}

#endif

// Mainloop
int main() {
  init();
  for(;;);
  return 0;
}

// EOF
