/*
 *  $Id$
 */

#include <stdio.h>

#include "debugio.h"
#include <avr/interrupt.h>

/*
 *  DEBUG_OUT_PORT  - -W0x20,/dev/stderr 
 *  DEBUG_IN_PORT   - -R0x22,/dev/stderr 
 */
#define DEBUG_OUT_PORT (*( (volatile char *)0x20))
#define DEBUG_IN_PORT (*( (volatile char *)0x22))

int debug_putchar(char c, FILE *stream)
{
  DEBUG_OUT_PORT = c;

  if (c == '\n')
    debug_putchar('\r', stream);
  return 0;
}

int debug_getchar(FILE *stream)
{
  return DEBUG_IN_PORT;
}

FILE debug_str =
  FDEV_SETUP_STREAM(debug_putchar, debug_getchar, _FDEV_SETUP_RW);

/*
 *  UART0 IO Support
 */
#include "uart.h"
#include "defines.h"

int uart_0_putchar(char c, FILE *stream)
{
  uart_0_putc(c);
}

int uart_0_getchar(FILE *stream)
{
  return uart_0_getc();
}

FILE uart0_str =
  FDEV_SETUP_STREAM(uart_0_putchar, uart_0_getchar, _FDEV_SETUP_RW);

/*
 *  UART1 IO Support
 */

int uart_1_putchar(char c, FILE *stream)
{
  uart_1_putc(c);
}

int uart_1_getchar(FILE *stream)
{
  return uart_1_getc();
}

FILE uart1_str =
  FDEV_SETUP_STREAM(uart_1_putchar, uart_1_getchar, _FDEV_SETUP_RW);

/*
 *  Initialize whatever we have to
 */
void debugio_init(void)
{
  uart_0_init(UART0_BAUD, F_CPU);
  uart_1_init(UART1_BAUD, F_CPU);
  sei();
}

