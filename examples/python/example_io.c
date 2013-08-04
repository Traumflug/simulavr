// modified from examples/atmega128_timer/main.c!
#include <avr/interrupt.h>

volatile int timer2_ticks;
volatile int port_val;
volatile int port_cnt;

ISR(TIMER2_COMP_vect) {
   timer2_ticks++;
}

int main(void) {
  volatile int tmp;
  int tmp2;
  
  DDRA = 0x01;
  PORTA = 0;

  /* Set up our timers and enable interrupts */
  TCNT2 = 0;   /* Timer 2 by CLK/64 */
  OCR2 = 124;  /* 2ms on 4MHz, CTC mode */
  TCCR2 = 0x0b;
  TIMSK = _BV(OCIE2);

  sei();

  tmp = timer2_ticks;
  port_cnt = 0;
  port_val = PINA & 0xfe;
  while(1) {
    if(tmp != timer2_ticks) { // toggle about every 2ms
      tmp = timer2_ticks;
      if((PINA & 0x01) == 0x01) {
        PORTA &= 0xfe;
      } else {
        PORTA |= 0x01;
      }
    }
    tmp2 = PINA & 0xfe;
    if(tmp2 != port_val) {
      port_val = tmp2;
      port_cnt++;
    }
  }

  return 0;
}
