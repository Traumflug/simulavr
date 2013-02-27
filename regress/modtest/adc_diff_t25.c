#include <avr/interrupt.h>

#include <avr/io.h>

volatile unsigned short adc_value = 0x5555;
volatile unsigned char complete = 1;
volatile unsigned char in_loop = 0;

void init(void) {
    // ADMUX: channel 6 (ADC2-ADC3), ADLAR = 0, REFS = 6 (2.56V)
    ADMUX = 0x96;
    // ADEN = 1, ADIE = 0, ADPS = 5 CKx32 = 125kHz
    ADCSRA = 0x85;
    // BIN = 1
    ADCSRB = 0x80;
}

int main(void) {

    init();

    do {
        in_loop = 1;
        switch(complete) {
            case 0:
                // wait until conversion complete, then get adc value
                if((ADCSRA & _BV(ADSC)) == 0) {
                    // read ADC value
                    adc_value = ADCL | (ADCH << 8);
                    complete = 1;
                }
                break;
            case 1:
                // conversion complete, idle
                break;
            case 2:
                // start conversion, channel 6
                ADMUX = 0x96;
                // set bipolar mode
                ADCSRB = 0x80;
                // start conversion
                ADCSRA |= _BV(ADSC);
                complete = 0;
                break;
            case 3:
                // start conversion, channel 6
                ADMUX = 0x96;
                // set unipolar mode
                ADCSRB = 0x00;
                // start conversion
                ADCSRA |= _BV(ADSC);
                complete = 0;
                break;
            case 4:
                // start conversion, channel 6
                ADMUX = 0x96;
                // set unipolar mode with IPR
                ADCSRB = 0x20;
                // start conversion
                ADCSRA |= _BV(ADSC);
                complete = 0;
                break;
        }
    } while(1); // do forever
}

// EOF
