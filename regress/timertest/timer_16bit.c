/* timer test programm for timer1: 16bit with normal mode */
#include <avr/interrupt.h>

volatile int timer_ticks;

#ifdef T3TEST
ISR(TIMER3_OVF_vect) {
   timer_ticks++;
}
ISR(TIMER3_COMPB_vect) {
   timer_ticks++;
}
#else
ISR(TIMER1_OVF_vect) {
   timer_ticks++;
}
#endif

int main(void) {
  volatile int tmp;

#ifdef T3TEST
  TCNT3H = 0;    /* Timer 3 by CLK/1 */
  TCNT3L = 0;
  OCR3AH = 0x06;
  OCR3AL = 0x3f; /* factor = 1600, PWM frequency = 5kHz */
  OCR3BH = 0x02;
  OCR3BL = 0x7f; /* duty factor 40% = 640 */
  DDRE   = 0x10; /* PE4 = out */
  TCCR3A = 0x30 + 0x03; /* comB = 3, wgm = 15 */
  TCCR3B = 0x18 + 0x01; /* wgm = 15, cs=1 */
  ETIMSK = _BV(TOIE3) + _BV(OCIE3B);
#else
  TCNT1H = 0;    /* Timer 1 by CLK/1 */
  TCNT1L = 0;
  TCCR1B = 0x01; /* 16,384ms on 4MHz, normal mode */
#ifdef PROC_atmega48
  TIMSK1 = _BV(TOIE1);
#else
  TIMSK = _BV(TOIE1);
#endif
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
