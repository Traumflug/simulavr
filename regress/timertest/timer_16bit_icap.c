/* timer test programm for timer1: 16bit with normal mode, use input capture interrupt */
#include <avr/interrupt.h>

volatile int timer_ticks;
volatile int input_capture;

ISR(TIMER1_OVF_vect) {
   timer_ticks++;
}

ISR(TIMER1_CAPT_vect) {
  input_capture = ICR1L + (ICR1H << 8);
}

int main(void) {
  volatile int tmp;

  TCNT1H = 0;    /* Timer 1 by CLK/1 */
  TCNT1L = 0;
#ifdef POS_EDGE_NO_C
  TCCR1B = 0x41; /* 16,384ms on 4MHz, normal mode, input capture on pos edge, no noise canceler */
#endif
#ifdef POS_EDGE_WITH_C
  TCCR1B = 0xc1; /* 16,384ms on 4MHz, normal mode, input capture on pos edge, no noise canceler */
#endif
#ifdef NEG_EDGE_NO_C
  TCCR1B = 0x01; /* 16,384ms on 4MHz, normal mode, input capture on pos edge, no noise canceler */
#endif
#ifdef NEG_EDGE_WITH_C
  TCCR1B = 0x81; /* 16,384ms on 4MHz, normal mode, input capture on pos edge, no noise canceler */
#endif
  TIMSK = _BV(TOIE1) + _BV(ICF1);

  sei();

  input_capture = 0;
  tmp = timer_ticks;
  while(1) {
    if(tmp != timer_ticks) {
      tmp = timer_ticks;
    } 
  }

  return 0;
}

/* EOF */
