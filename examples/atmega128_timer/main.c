#include <avr/interrupt.h>
#include <stdio.h>
#include "debugio.h"

volatile int timer2_ticks;

/* Every ~ms */
ISR(TIMER2_COMP_vect)
{                               /* Every ~1ms */
   timer2_ticks++;
}

int main(
  int argc,
  char **argv
)
{
  volatile int tmp;

  debugio_init();

  /* Set up our timers and enable interrupts */
  TCNT2 = 0;		/* Timer 2 by CLK/64 */
  OCR2 = 115;		/* ~1ms */
  TCCR2 = 0x0b;
  TIMSK = _BV(OCIE2);

  sei();

  tmp = timer2_ticks;
  while(tmp <= 500) {
    if ( tmp != timer2_ticks ) {
      printk( "%d ", timer2_ticks );
      tmp = timer2_ticks;
    } 
  }

  return 0;
}
