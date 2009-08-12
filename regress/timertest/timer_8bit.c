/* timer test programm for timer2: 8bit with different modes */
#include <avr/interrupt.h>

volatile int timer2_ticks;

#if TEST_MODE==1
ISR(SIG_OVERFLOW2) {
   timer2_ticks++;
}
#endif

int main(void) {
  volatile int tmp;

  DDRA = 0x01;
  PORTA = 0;

#if TEST_MODE==1
  TCNT2 = 0;    /* Timer 2 by CLK/8 */
  TCCR2 = 0x02; /* 512us on 4MHz, normal mode */
  TIMSK = _BV(TOIE2);
#endif

  sei();

  tmp = timer2_ticks;
  while(1) {
    if(tmp != timer2_ticks) { /* toggle PORTA0 every time timer2_ticks is changed */
      tmp = timer2_ticks;
      if((PINA & 0x01) == 0x01) {
        PORTA &= 0xfe;
      } else {
        PORTA |= 0x01;
      }
    } 
  }

  return 0;
}

/* EOF */
