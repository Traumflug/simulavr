#include <avr/interrupt.h>

#include <avr/io.h>

volatile unsigned short adc_value = 0;
volatile unsigned char complete = 0;
volatile unsigned char in_loop = 0;

#if defined(PROC_at90s4433) || defined(PROC_atmega8)
#define CTLREG ADCSR
#else
#define CTLREG ADCSRA
#endif

void init(void) {
#ifdef PROC_attiny25
    // ADMUX: channel 0, REF=5 (1 would also be possible!)
    ADMUX = 0x50;
#else
    // ADMUX: channel 0, ADCBG = 0 (at90s4433 only)
    ADMUX = 0x00;
#endif
    // ADEN = 1, ADIE = 1, ADPS = 5 CKx32 = 125kHz
    CTLREG = 0x8d;
}

ISR(ADC_vect) {
    // read ADC value
    adc_value = ADCL | (ADCH << 8);    
    complete = 1;
}

int main(void) {

    init();

    // start ADC
    complete = 0;
    CTLREG = 0xcd;

    // enable interrupts
    sei();

    do {
        in_loop = 1;
        if(complete == 2) {
            // start next conversion
            complete = 0;
            CTLREG |= _BV(ADSC);
        }
    } while(1); // do forever
}

// EOF
