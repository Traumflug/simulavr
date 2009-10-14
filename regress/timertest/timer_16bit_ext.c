/* timer test programm for timer1: 16bit with external clock */
#include <avr/interrupt.h>

volatile int timer_ticks;

int main(void) {
  int tmp, tmp2;

  TCNT1H = 0;    /* Timer 1 with external clock */
  TCNT1L = 0;
  TCCR1A = 0;
#ifdef NEGEDGE_T1
  TCCR1B = 0x06; /* normal mode, external clock negative edge */
#else
  TCCR1B = 0x07; /* normal mode, external clock positive edge */
#endif
  TCCR1C = 0; 

  tmp = 0;
  tmp2 = 0;
  while(1) {
    tmp2 = TCNT1L;
    tmp2 += (TCNT1H << 8);
    if(tmp != tmp2) {
      tmp = tmp2;
      timer_ticks = tmp;
    } 
  }

  return 0;
}

/* EOF */
