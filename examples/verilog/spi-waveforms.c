/*
** Copyright (C) 2009 Onno Kortmann <onno@gmx.net>
**  
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**  
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**  
** You should have received a copy of the GNU General Public License along
** with this program; if not, write to the Free Software Foundation, Inc.,
** 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA..
**  
*/
#include <avr/io.h>

#define SPI_CS      2
#define SPI_MOSI    3
#define SPI_MISO    4
#define SPI_SCK     5

void spitxdel() {
    while(!(SPSR & (1<<SPIF))); SPDR;
    // 'slave reset'
    PORTB |= _BV(SPI_CS);
    PORTB &= ~_BV(SPI_CS);
}


int main() {
    DDRB |= (1<<SPI_CS);
    DDRB |= (1<<SPI_MOSI);	
    DDRB &=~(1<<SPI_MISO);
    DDRB |= (1<<SPI_SCK);

    // 'slave reset'
    PORTB |= _BV(SPI_CS);
    PORTB &= ~_BV(SPI_CS);

    // standard options
    SPCR = (1<<SPE)|(1<<MSTR)|(1<<DORD);
    SPDR = 0xa5;
    spitxdel();
    
    
    // two times rate
    SPSR |= (1<<SPI2X);
    SPDR = 0xa5;
    spitxdel();
    
    // MSB first
    SPCR = (1<<SPE)|(1<<MSTR);
    SPDR = 0xa5;
    spitxdel();


    // inversed clock
    SPCR = (1<<SPE)|(1<<MSTR)|(1<<CPOL);
    SPDR = 0xa5;
    spitxdel();


    // other phase
    SPCR = (1<<SPE)|(1<<MSTR)|(1<<CPHA);
    SPDR = 0xa5;
    spitxdel();
}
