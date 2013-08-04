#include <avr/interrupt.h>

#if defined(__AVR_ATtiny2313__)
# define FLAGREG EIFR
# define MASKREG GIMSK
# define FLAGBIT _BV(PCIF)
# define PORTREG PINB
# define CTRLREG PCMSK
#endif

#if defined(__AVR_ATmega48__)
# define FLAGREG PCIFR
# define MASKREG PCICR
# define FLAGBIT _BV(PCIF1)
# define PORTREG PINC
# define CTRLREG PCMSK1
#endif

volatile unsigned char cnt_irq = 0;
volatile unsigned char hs_in = 0;
volatile unsigned char hs_out = 0;
volatile unsigned char hs_cmd = 0;
volatile unsigned char hs_data = 0;

#if defined(__AVR_ATtiny2313__)
  ISR(PCINT_vect) {
#endif
#if defined(__AVR_ATmega48__)
  ISR(PCINT1_vect) {
#endif
  cnt_irq++;
}

int main() {
  
  while(1) {
    
    // do we have a new command?
    if(hs_in != hs_out) {
      switch(hs_cmd) {
        
        default:
          // noop
          break;
          
        case 1:
          // sei or cli
          if(hs_data != 0) {
            cli();
            MASKREG |= FLAGBIT;
            sei();
          } else {
            cli();
            MASKREG &= ~FLAGBIT;
          }
          break;
          
        case 2:
          // clear interrupt counter, this is a atomic op, so we don't need to
          // disable interrupt
          cnt_irq = 0;
          break;
          
        case 3:
          // reset flag bit
          FLAGREG |= FLAGBIT;
          break;
          
        case 4:
          // set control register
          CTRLREG = hs_data;
          break;
          
        case 5:
          // get port input
          hs_data = PORTREG;
          break;
          
        case 6:
          // get flag register
          hs_data = FLAGREG;
          break;
          
        case 7:
          // get mask register
          hs_data = MASKREG;
          break;
          
      }
      // clear command
      hs_cmd = 0;
      // handshake
      hs_out = hs_in;
    }
    
  } // this will never return
  
  return 0;
  
}
