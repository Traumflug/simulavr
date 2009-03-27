/*
 *  $Id$
 */

#include <stdio.h>

#include "debugio.h"

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
  return DEBUG_OUT_PORT;
}

FILE debug_str =
  FDEV_SETUP_STREAM(debug_putchar, debug_getchar, _FDEV_SETUP_RW);

/*
 *  UART0 IO Support
 */
#include "uart.h"

FILE uart0_str =
  FDEV_SETUP_STREAM(uart_putchar, uart_getchar, _FDEV_SETUP_RW);

/*
 *  Initialize whatever we have to
 */
void debugio_init(void)
{
  printk( "Initialize debug io\n" );
  uart_init();
}

