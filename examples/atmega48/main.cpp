/* 
 * Demonstrate the use of the SpiSource, SpiSink, and PinMonitor.
 */

#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdint.h>

uint8_t				txData;
volatile uint8_t	rxData;
uint8_t				mux;

static void startADC()
{
	uint8_t	value;

	value	= ADMUX;
	value	&= ~(	(0<<MUX3)
				|	(0<<MUX2)
				|	(0<<MUX1)
				|	(0<<MUX0)
				);

	switch(mux){
		case 0:
			value	|= 0x05;
		case 1:
			value	|= 0x06;
		case 2:
			value	|= 0x07;
		};

	ADMUX	= value;

	++mux;

	if(mux > 2){
		mux	= 0;
		}


	ADCSRA	=	(	(1<<ADEN)	// ADC Enabled
				|	(1<<ADSC)	// Start Conversion NOW
				|	(1<<ADIF)	// Interrupt Flag Acknowledge
				|	(1<<ADIE)	// Interrupt enabled
				|	(0<<ADPS2)	// Prescaler should get between 50KHz-200KHz for max resolution
				|	(1<<ADPS1)	//   System Clock = 1MHz
				|	(1<<ADPS0)	//		Prescaler = 1MHz/100KHz = ~10 thus = 8 : 1MHz/8 = 120KHz
				) ;
}

static void	assertPB0(){
	// asserted is LOW
	PORTB	&= ~(1<<PB0);
	}

static void	negatePB0(){
	// negated is HIG
	PORTB	|= (1<<PB0);
	}

uint8_t	count;
uint8_t	adcData;

ISR(ADC_vect)
{
	adcData	= ADCH;
	startADC();
}

ISR(SPI_STC_vect)
{
	rxData	= SPDR;
	if(count % 2){
		SPDR	= rxData;
		}
	else {
		SPDR	= adcData;
		}

	if(count == 0){
		assertPB0();
		}

	if(count == 128){
		negatePB0();
		}
	++count;
}

int main(int argc,char *argv[]){
	ADMUX	=		(0<<REFS1)	// Use Vcc as ADC reference
				|	(1<<REFS0)	// Use Vcc as ADC reference
				|	(1<<ADLAR)	// Left justify result (percentage FS)
				|	(0<<MUX3)
				|	(0<<MUX2)
				|	(0<<MUX1)
				|	(0<<MUX0)
				;

	PORTB	=		(1<<PB0)	// Interrupt output
				|	(0<<PB2)	// /SS input
				|	(0<<PB3)	// MOSI input
				|	(1<<PB4)	// MISO output
				|	(0<<PB5)	// SCK input
				;

	DDRB	=		(1<<PB0)	// Interrupt output
				|	(0<<PB2)	// /SS input
				|	(0<<PB3)	// MOSI input
				|	(1<<PB4)	// MISO output
				|	(0<<PB5)	// SCK input
				;

	SPCR	=		(1<<SPIE)	// interrupt enable
				|	(1<<SPE)	// SPI enable
				|	(0<<DORD)	// MSB first
				|	(0<<MSTR)	// Slave Mode
				|	(1<<CPOL)	// Clock HIGH when idle
				|	(0<<CPHA)	// Sample on leading edge
				|	(0<<SPR1)	// Slave has no affect
				|	(0<<SPR0)	// Slave has no affect
				;

	startADC();

	sei();

	for(;;);

    return 0;
}




