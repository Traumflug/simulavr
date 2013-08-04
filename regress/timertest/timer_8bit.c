/* timer test programm for timer0: 8bit with normal mode */
#include <avr/interrupt.h>

volatile int timer_ticks;

ISR(TIMER0_OVF_vect) {
   timer_ticks++;
}

int main(void) {
  volatile int tmp;

  TCNT0 = 0;    /* Timer 0 by CLK/8 */
#if defined PROC_atmega48
  TCCR0B = 0x02; /* 512us on 4MHz, normal mode */
  TIMSK0 = _BV(TOIE0);
#elif defined PROC_attiny2313 || defined PROC_attiny25
  TCCR0B = 0x02;
  TIMSK = _BV(TOIE0);
#else
  TCCR0 = 0x02;
  TIMSK = _BV(TOIE0);
#endif

  sei();

  tmp = timer_ticks;
  while(1) {
    if(tmp != timer_ticks) {
      tmp = timer_ticks;
    } 
  }

  return 0;
}

/* EOF */
