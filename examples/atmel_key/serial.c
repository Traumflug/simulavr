/*
 *  $Id$
 */

#include <avr/io.h>
#include <stdlib.h>
//include <sig-avr.h>
#include <avr/interrupt.h>

#include "serial.h"
#include "StdDefs.h"


// 02-Sep-2008 K. Schwichtenberg Clearing of UDRIE corrected

#define ESC 0x1b
#define BUFF_SIZE 64

unsigned char CLR[] = {ESC, '[','H', ESC, '[', '2', 'J',0};

unsigned char UART_buffer[BUFF_SIZE];
unsigned char *inptr, *outptr;
unsigned char buff_cnt;


void init_uart(void)
{
    inptr =  UART_buffer;
    outptr = UART_buffer;
    buff_cnt = 0;
}

void clr(void)
{
    //putstr(CLR);                                // Send a 'clear screen' to a VT100 terminal
}


int putchar(int c)
{
    if (buff_cnt<BUFF_SIZE)
    {
        *inptr = c;                             // Put character into buffer
        inptr++;                                // Increment pointer

        buff_cnt++;

        if (inptr >= UART_buffer + BUFF_SIZE)   // Pointer wrapping
            inptr = UART_buffer;

        UART_CONTROL_REG = 0x38;                // Enable UART Data register
                                                // empty interrupt

        return 1;
    } else {
        return 0;                               // Buffer is full
    }

}

// Interrupt driven transmitter

SIGNAL(UART_REG_EMPTY_INT_VECTOR)
{
    UART_DATA_REG = *outptr;                    // Send next byte
    outptr++;                                   // Increment pointer

    if (outptr >= UART_buffer + BUFF_SIZE)      // Pointer wrapping
        outptr = UART_buffer;

    if(--buff_cnt == 0)                         // If buffer is empty:
//        UART_CONTROL_REG = UART_CONTROL_REG && (1<<UDRIE); // disable interrupt This was Atmels original
        UART_CONTROL_REG = UART_CONTROL_REG & (~(1<<UDRIE)); // disable interrupt This is what they meant K.Schwi
}

