/*
 *  $Id$
 */

#include <avr/io.h>
#include <stdlib.h>
//include <sig-avr.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "kb.h"
#include "serial.h"
#include "pindefs.h"
#include "scancodes.h"
#include "StdDefs.h"

#define BUFF_SIZE 64

// 02-Sep-2008 K. Schwichtenberg Timing modified for the avr-libc _delay_ms

unsigned char edge, bitcount;                // 0 = neg.  1 = pos.

unsigned char kb_buffer[BUFF_SIZE];
unsigned char *inpt, *outpt;
unsigned char buffcnt;


void init_kb(void)
{
    inpt =  kb_buffer;                        // Initialize buffer
    outpt = kb_buffer;
    buffcnt = 0;

    MCUCR = 2;                                // INT0 interrupt on falling edge
    edge = 0;                                // 0 = falling edge  1 = rising edge
    bitcount = 11;
}

SIGNAL(INT0_vect)
{
    static unsigned char data;                // Holds the received scan code

    if (!edge)                                // Routine entered at falling edge
    {
        if(bitcount < 11 && bitcount > 2)    // Bit 3 to 10 is data. Parity bit,
        {                                    // start and stop bits are ignored.
            data = (data >> 1);
            if(PIND & 8)
                data = data | 0x80;            // Store a '1'
        }

        MCUCR = 3;                            // Set interrupt on rising edge
        edge = 1;

    } else {                                // Routine entered at rising edge

        MCUCR = 2;                            // Set interrupt on falling edge
        edge = 0;

        if(--bitcount == 0)                    // All bits received
        {
            decode(data);
            bitcount = 11;
        }
    }
    //    test_pin();
}


void decode(unsigned char sc)
{
    static unsigned char is_up=0, shift = 0, mode = 0;
    unsigned char i;

    if (!is_up)                // Last data received was the up-key identifier
    {
        switch (sc)
        {
          case 0xF0 :        // The up-key identifier
            is_up = 1;
            break;

          case 0x12 :        // Left SHIFT
            shift = 1;
            break;

          case 0x59 :        // Right SHIFT
            shift = 1;
            break;

          case 0x05 :        // F1
            if(mode == 0)
                mode = 1;    // Enter scan code mode
            if(mode == 2)
                mode = 3;    // Leave scan code mode
            break;

          default:
            if(mode == 0 || mode == 3)        // If ASCII mode
            {
                if(!shift)                    // If shift not pressed,
                {                            // do a table look-up
                    for(i = 0; pgm_read_byte(&unshifted[i][0])!=sc
                            && pgm_read_byte(&unshifted[i][0]); i++);
                    if (pgm_read_byte(&unshifted[i][0]) == sc) {
                        put_kbbuff(pgm_read_byte(&unshifted[i][1]));
                    }
                } else {                    // If shift pressed
                    for(i = 0; pgm_read_byte(&shifted[i][0])!=sc && pgm_read_byte(&shifted[i][0]); i++);
                    if (pgm_read_byte(&shifted[i][0]) == sc) {
                        put_kbbuff(pgm_read_byte(&shifted[i][1]));
                    }
                }
            } else{                            // Scan code mode
                print_hexbyte(sc);            // Print scan code
                put_kbbuff(' ');
                put_kbbuff(' ');
            }
            break;

        }
    } else {
        is_up = 0;                            // Two 0xF0 in a row not allowed
        switch (sc)
        {
          case 0x12 :                        // Left SHIFT
            shift = 0;
            break;

          case 0x59 :                        // Right SHIFT
            shift = 0;
            break;

          case 0x05 :                        // F1
            if(mode == 1)
                mode = 2;
            if(mode == 3)
                mode = 0;
            break;
          case 0x06 :                        // F2
            clr();
            break;
        }
    }
}

void put_kbbuff(unsigned char c)
{
    if (buffcnt<BUFF_SIZE)                        // If buffer not full
    {
        *inpt = c;                                // Put character into buffer
        inpt++;                                    // Increment pointer

        buffcnt++;

        if (inpt >= kb_buffer + BUFF_SIZE)        // Pointer wrapping
            inpt = kb_buffer;
    }
}

int getchar(void)
{
    int byte;
    while(buffcnt == 0)     {     // Wait for data
        run_led(3000, 3000);     // Timing changed / 10 K.Schwi
    }
    byte = *outpt;                                // Get byte
    outpt++;                                    // Increment pointer

    if (outpt >= kb_buffer + BUFF_SIZE)            // Pointer wrapping
        outpt = kb_buffer;

    buffcnt--;                                    // Decrement buffer count

    return byte;
}

