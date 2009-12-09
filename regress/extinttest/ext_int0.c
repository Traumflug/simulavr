#include <avr/interrupt.h>

#if defined(__AVR_ATmega128__)
# define FLAGREG EIFR
# define MASKREG EIMSK
# if defined(TEST_INT1)
#   define FLAGBIT _BV(INTF1)
# else
#   define FLAGBIT _BV(INTF0)
# endif
# define PORTREG PIND
# define CTRLREG EICRA
#endif

#if defined(__AVR_ATmega16__)
# define FLAGREG GIFR
# define MASKREG GICR
# if defined(TEST_INT1)
#   define FLAGBIT _BV(INTF1)
# else
#   define FLAGBIT _BV(INTF0)
# endif
# define PORTREG PIND
# define CTRLREG MCUCR
#endif

#if defined(__AVR_ATmega48__)
# define FLAGREG EIFR
# define MASKREG EIMSK
# if defined(TEST_INT1)
#   define FLAGBIT _BV(INTF1)
# else
#   define FLAGBIT _BV(INTF0)
# endif
# define PORTREG PIND
# define CTRLREG EICRA
#endif

#if defined(__AVR_ATtiny2313__)
# define FLAGREG EIFR
# define MASKREG GIMSK
# if defined(TEST_INT1)
#   define FLAGBIT _BV(INTF1)
# else
#   define FLAGBIT _BV(INTF0)
# endif
# define PORTREG PIND
# define CTRLREG MCUCR
#endif

volatile unsigned char cnt_irq = 0;
volatile unsigned char hs_in = 0;
volatile unsigned char hs_out = 0;
volatile unsigned char hs_cmd = 0;
volatile unsigned char hs_data = 0;
volatile unsigned char dis_mask = 0;

#if defined(TEST_INT1)
  ISR(SIG_INTERRUPT1) {
#else
  ISR(SIG_INTERRUPT0) {
#endif
  cnt_irq++;
  if(dis_mask != 0)
    MASKREG &= ~FLAGBIT;
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
