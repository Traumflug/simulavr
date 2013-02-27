/* this code is taken from examples/anacomp */
#include <avr/interrupt.h>

#include <avr/io.h>

volatile unsigned char in_loop = 0;
volatile unsigned char loop_count = 0;

// PORT A
#define init_a() { DDRA = 0x02; }
#define update_a() { PORTA = (PINA & 0x1) << 1; }
// PORT B
#define init_b() { DDRB = 0x02; }
#define update_b() { PORTB = (PINB & 0x1) << 1; }
// PORT C
#define init_c() { DDRC = 0x02; }
#define update_c() { PORTC = (PINC & 0x1) << 1; }
// PORT D
#define init_d() { DDRD = 0x02; }
#define update_d() { PORTD = (PIND & 0x1) << 1; }
// PORT E
#define init_e() { DDRE = 0x02; }
#define update_e() { PORTE = (PINE & 0x1) << 1; }
// PORT F
#define init_f() { DDRF = 0x02; }
#define update_f() { PORTF = (PINF & 0x1) << 1; }
// PORT G
#define init_g() { DDRG = 0x02; }
#define update_g() { PORTG = (PING & 0x1) << 1; }

void init(void) {
#if !defined(PROC_at90s4433) && !defined(PROC_atmega8) && !defined(PROC_atmega48) && !defined(PROC_attiny25)
  init_a();
#endif
  init_b();
#if !defined(PROC_attiny2313) && !defined(PROC_attiny25)
  init_c();
#endif
#if !defined(PROC_attiny25)
  init_d();
#endif
#if defined(PROC_atmega64) || defined(PROC_atmega128) || defined(PROC_at90can32) 
  init_e();
  init_f();
  init_g();
#endif
}

int main(void) {

    init();

    do {
        in_loop = 1;

#if !defined(PROC_at90s4433) && !defined(PROC_atmega8) && !defined(PROC_atmega48) && !defined(PROC_attiny25)
        update_a();
#endif
        update_b();
#if !defined(PROC_attiny2313) && !defined(PROC_attiny25)
        update_c();
#endif
#if !defined(PROC_attiny25)
        update_d();
#endif
#if defined(PROC_atmega64) || defined(PROC_atmega128) || defined(PROC_at90can32) 
        update_e();
        update_f();
        update_g();
#endif

        loop_count++;
    } while(1); // do forever
}

// EOF
