/* 
 * Demonstrate the use of the SpiSource, SpiSink, and PinMonitor.
 *
 * $Id$
 */

#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdint.h>

uint8_t				txData;
volatile uint8_t	rxData;

static void startADC()
{
	ADCSRA	=	(	(1<<ADEN)	// ADC Enabled
				|	(1<<ADSC)	// Start Conversion NOW
				|	(0<<ADFR)	// Auto Trigger Disabled
				|	(1<<ADIF)	// Interrupt Flag Acknowledge
				|	(1<<ADIE)	// Interrupt enabled
				|	(0<<ADPS2)	// Prescaler should get between 50KHz-200KHz for max resolution
				|	(1<<ADPS1)	//   System Clock = 1MHz
				|	(1<<ADPS0)	//		Prescaler = 1MHz/100KHz = ~10 thus = 8 : 1MHz/8 = 120KHz
				) ;
}

static void	assertPA0(){
	// asserted is LOW
	PORTA	&= ~(1<<PA0);
	}

static void	negatePA0(){
	// negated is HIG
	PORTA	|= (1<<PA0);
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
		assertPA0();
		}

	if(count == 128){
		negatePA0();
		}
	++count;
}

int main(int argc,char *argv[]){
	ADMUX	=		(0<<REFS1)	// Use Vcc as ADC reference
				|	(1<<REFS0)	// Use Vcc as ADC reference
				|	(1<<ADLAR)	// Left justify result (percentage FS)
				|	(0<<MUX4)	// Always zero for our application
				|	(0<<MUX3)
				|	(0<<MUX2)
				|	(0<<MUX1)
				|	(0<<MUX0)
				;

	DDRB	&= ~(	(1<<PB0)	// /SS PortB pin 0 as input
				|	(1<<PB1)	// SCK PortB pin 1 as input for slave
				|	(1<<PB2)	// MOSI PortB pin 2 as input for slave
				);
	DDRB	|=	(1<<PB3);		// MISO PortB pin 2 as output for slave
	PORTA	|=	(1<<PA0);		// Negate "interrupt"
	DDRA	|=  (1<<PA0);		// "Interrupt" output

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




