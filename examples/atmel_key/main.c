/*
 *  $Id$
 */

// PS-1 Keyboard interface test program

#include <avr/io.h>
#include <stdlib.h>
//include <sig-avr.h>
#include <avr/interrupt.h>
#include <compat/deprecated.h>

#include "serial.h"
#include "kb.h"
#include "StdDefs.h"

// 02-Sep-2008 K. Schwichtenberg Enable INT0 interrupt changed into portable code
// 02-Sep-2008 K. Schwichtenberg Workaround for weakness of simulavrxx (TxD0-Pin set to output)

void initialize(void);


int main(void)
{
    unsigned char key;

    initialize();
    putchar('I');

    while(1)
    {
        key=getchar();
        putchar(key);
        msleep(10);
    }
    return 1;
}

void initialize(void)
   {
   cli();

   PORTB = 0xFD;
   DDRB = 0x02;     // Port B pin 1 as test pin
   TESTPIN_OFF();

   PORTD = 0x5F;
   DDRD = 0xA0;     // All inputs with pullups.  UART will override.
                    // Pin6 - Out as RunLED, Pin7-out as RF module power

   PORTE = 0x02;
   DDRE = 0x02;   // TxD0 set to output (simulavrxx weakness K. Schwichtenberg)

   init_kb();
   init_uart();

   UART_CONTROL_REG = 0x18;   //Transmitter enabled, receiver enabled, no ints
   setbaud(BAUD19K);

   EIMSK= (1<<INT0);        // Enable INT0 interrupt
// EIMSK= 0x40; That might be okay for M163 but is not portable   K.Schwichtenberg

   //putchar('I');

   sei();
   }

