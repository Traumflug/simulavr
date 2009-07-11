#include <avr/io.h>


#define SPI_CS      2
#define SPI_MOSI    3
#define SPI_MISO    4
#define SPI_SCK     5

int main() {
    DDRB |= (1<<SPI_CS);
    DDRB |= (1<<SPI_MOSI);	
    DDRB &=~(1<<SPI_MISO);
    DDRB |= (1<<SPI_SCK);

    // reset slave
    PORTB |= _BV(SPI_CS);
    PORTB &= ~_BV(SPI_CS);

    SPCR = (1<<SPE)|(1<<MSTR)|(1<<DORD)|(1<<CPHA)|(1<<CPOL);
    
    SPDR = 0x35;
    while(!(SPSR & (1<<SPIF)));

    SPDR = 0x12;
    while(!(SPSR & (1<<SPIF)));
    
    SPDR = 0x01;
    while(!(SPSR & (1<<SPIF)));

    SPDR = 0x56;
    while(!(SPSR & (1<<SPIF)));

    
    SPDR = 0x35;
    while(!(SPSR & (1<<SPIF)));

    SPDR = 0x12;
    while(!(SPSR & (1<<SPIF)));
    
    SPDR = 0x00;
    while(!(SPSR & (1<<SPIF)));

    SPDR = 0x00;
    while(!(SPSR & (1<<SPIF))); SPDR=SPDR;
    while(!(SPSR & (1<<SPIF)));
    
}
