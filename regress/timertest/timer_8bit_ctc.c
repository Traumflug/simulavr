/* timer test programm for timer2: 8bit with ctc mode */
#include <avr/interrupt.h>

volatile int timer_ticks;

#ifdef PROC_atmega48
ISR(TIMER2_COMPA_vect) {
   timer_ticks++;
}
#else
ISR(TIMER2_COMP_vect) {
   timer_ticks++;
}
#endif

int main(void) {
  volatile int tmp;

  TCNT2 = 0;    /* Timer 2 by CLK/64 */
#ifdef PROC_atmega48
  OCR2A = 124;  /* 2ms on 4MHz, CTC mode */
  TCCR2A = 0x40 + 0x02; /* comA = 1, wgm=2 */
  TCCR2B = 0x04; /* cs=4 */
  DDRB = 0x08; /* PB3=out */
  TIMSK2 = _BV(OCIE2A);
#else
  OCR2 = 124;  /* 2ms on 4MHz, CTC mode */
  TCCR2 = 0x08 + 0x03 + 0x10; /* wgm=2, cs=3, com=1 */
  DDRB = 0x80; /* PB7=out */
  TIMSK = _BV(OCIE2);
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
