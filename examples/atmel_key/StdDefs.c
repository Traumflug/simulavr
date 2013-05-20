/*
 *  $Id$
 */

// Common routines used in most programs

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <string.h>
#include <compat/deprecated.h>

#include "StdDefs.h"
#include "serial.h"

// 02-Sep-2008 K. Schwichtenberg _delay_ms included for a useful timing

void setbaud(BaudRate br)
   {
   UART_BAUD_REG = br;
   }

// Change State every on or off calls to run_led
// Define on time and off time
void run_led(INT16U ontime, INT16U offtime)
    {
	static int c;
   static int x;

   if (c > x)
	   {
	   if (bit_is_set(RUNLED_PORT, RUNLED_BIT))
         {
         cbi(RUNLED_PORT, RUNLED_BIT);
         x = offtime;
         }
	   else
	 	  {
         sbi(RUNLED_PORT, RUNLED_BIT);
         x = ontime;
         }
	   c = 0;
	   }
	else
	   c++;
    }

// Toggles the test pin every call
void test_pin(void)
   {
   if (bit_is_set(TESTPIN_PORT, TESTPIN_BIT))
       cbi(TESTPIN_PORT, TESTPIN_BIT);
   else
       sbi(TESTPIN_PORT, TESTPIN_BIT);
   }

// Delay in 1/10's of a millisecond
// Does not work with -O0, use -O1, even for debugging.
void msleep(INT16U ms10)
    {
        for( ; ms10; --ms10)
            _delay_ms(0.1);
    }

//------------------------------------------------------------
// void putBCD(INT16S X, CHARU length, CHARU TrailingSpace)
//
// Outputs to UART a signed integer represented as a decimal integer
// up to 5 digits long plus negative sign plus trailing space (-)DDDD.
// Prefixes a - sign if number is negative.
// If TrailingSpace is not equal to 0 a trailing space is appended
// Length defines number of characters printed including - sign and
// trailing space.

void putBCD(INT16S X, CHARU length, CHARU TrailingSpace)

	 {
	  CHARU byte1, byte2, byte3, byte4, byte5;

     if (TrailingSpace) length --;

     if (X < 0)
       {
       X = X * (-1);
       putchar('-');
       length --;
       }

	  byte1 = (CHARU)(X % 10) + 0x30;
	  X /= 10;
	  byte2 = (CHARU)(X % 10) + 0x30;
	  X /= 10;
	  byte3 = (CHARU)(X % 10) + 0x30;
	  X /= 10;
	  byte4 = (CHARU)(X % 10) + 0x30;
	  X /= 10;
	  byte5 = (CHARU)(X % 10) + 0x30;

	  if (length > 4) putchar(byte5);
	  if (length > 3) putchar(byte4);
	  if (length > 2) putchar(byte3);
	  if (length > 1) putchar(byte2);
	  putchar(byte1);
     if (TrailingSpace) putchar(' ');

    }
/*
void putchar(CHARU c)
   {
   while(bit_is_clear(UART_STATUS_REG, 5));
   UART_DATA_REG = c;
   }
*/

void putstr(CHARU *s)
   {
   int j;
   for (j = 0; j <= strlen((char*)s); j++)
       putchar(*(s + j));
   CRLF();
   }

void print_hexbyte(unsigned char i)
{
    unsigned char h, l;

    h = i & 0xF0;               // High nibble
    h = h>>4;
    h = h + '0';

    if (h > '9')
        h = h + 7;

    l = (i & 0x0F)+'0';         // Low nibble
    if (l > '9')
        l = l + 7;


    putchar(h);
    putchar(l);
}

