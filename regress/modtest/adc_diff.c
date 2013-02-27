#include <avr/interrupt.h>

#include <avr/io.h>

volatile unsigned short adc_value = 0x5555;
volatile unsigned char complete = 0;
volatile unsigned char in_loop = 0;

void init(void) {
#ifdef ADC_GAIN200
    // ADMUX: channel 10, ADLAR = 0, REFS = 3
    ADMUX = 0xca;
#else
    // ADMUX: channel 18, ADLAR = 0, REFS = 3
    ADMUX = 0xd2;
#endif
    // ADEN = 1, ADIE = 0, ADPS = 5 CKx32 = 125kHz
    ADCSRA = 0x85;
}

int main(void) {

    init();

    // start ADC
    ADCSRA = 0xc5;

    do {
        in_loop = 1;
        if((complete == 0) && ((ADCSRA & _BV(ADSC)) == 0)) {
            // read ADC value
            adc_value = ADCL | (ADCH << 8);
            complete = 1;
        }
        if(complete == 2) {
#ifdef ADC_GAIN200
            // change channel to channel 11
            ADMUX = 0xcb;
#else
            // change channel to channel 19
            ADMUX = 0xd3;
#endif 
            // start next conversion
            complete = 0;
            ADCSRA |= _BV(ADSC);
        }
    } while(1); // do forever
}

// EOF
