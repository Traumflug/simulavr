#include <avr/interrupt.h>

#include <avr/io.h>
#include <avr/eeprom.h>

volatile unsigned char in_loop = 0;
volatile unsigned char complete = 1;

typedef struct {
    unsigned char dummy; // Addr. 0, don't use
    unsigned char byte1;
} eep_t;
eep_t EEMEM eep = { 0, 0x33 };

#if defined(PROC_attiny25) || defined(PROC_atmega8) || defined(PROC_at90s4433)
ISR(EE_RDY_vect) {
#elif defined(PROC_attiny2313)
ISR(EEPROM_READY_vect) {
#else
ISR(EE_READY_vect) {
#endif
    // EEPROM cell written, reset EERIE
    EECR &= ~_BV(EERIE);
    complete = 1;
}

int main(void) {

    sei();

    do {
        in_loop = 1;
        if(complete == 2) {
            EEDR = 0x66;
            EEARL = 0x01;
#if !defined(PROC_at90s4433) && !defined(PROC_attiny2313)
            EEARH = 0x00;
#endif
            EECR = 0x0c; // set EERIE and EEMWE
            EECR = 0x0a; // set EERIE and EEWE
            complete = 0;
        }
    } while(1); // do forever
}

// EOF
