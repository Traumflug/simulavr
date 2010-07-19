#include <avr/io.h>
#include <avr/interrupt.h>

volatile uint32_t dummy = 0;
volatile uint32_t conversions = 0;

volatile uint16_t adc_value = 5555;
ISR(ADC_vect)
{
    if((ADCSRA & (1 << ADSC)) == 0)
    {
        ++conversions;
        adc_value = ADC;
        ADCSRA |= /* ADC Control and Status Register A */
            (1 << ADSC); /* ADC Start Conversion */
    }
}

int main(void)
{
    /* I'm not sure if this is necessary. I think, program startup is
     * considered to be handling of the reset interrupt.
     * So Interrupts will be disabled at program startup.
     * just to be on the safe side: */
    cli();

    ADCSRA = /* ADC Control and Status Register A */
          (1 << ADEN) /* ADC Enable */
        | (1 << ADSC) /* ADC Start Conversion */
        | (1 << ADIE) /* ADC Interrupt Enable */
        | (1<< ADPS2) /* ADPS2...ADPS0 ADC Prescaler Select Bits = 1 0 0, on 4MHz,
                         factor 16 = 4µs adc clock, conversion time x13 = 52µs normally */
        ;
    ADMUX = /* ADC Multiplexer Select Register */
        /* REFS1...REFS0 (ReferenceSelection Bits) = 0 --> Externes AREF */        
        (0 << MUX2) | (0 << MUX1) | (0 << MUX0); /* MUX4...MUX0 = channel*/

    sei();
    while(1)
    {
        ++dummy;
    }
}
