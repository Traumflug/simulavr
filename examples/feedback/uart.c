/*************************************************************************
DESCRIPTION:
    An interrupt is generated when the UART has finished transmitting or
    receiving a byte. The interrupt handling routines use circular buffers
    for buffering received and transmitted data.

    The UART_RX_BUFFER_SIZE and UART_TX_BUFFER_SIZE variables define
    the buffer size in bytes. Note that these variables must be a
    power of 2.

USAGE:
    Refere to the header file uart.h for a description of the routines.
    See also example test_uart.c.

NOTES:
    Based on Atmel Application Note AVR306

  $Id$

*************************************************************************/

#include <avr/io.h>
#include <avr/interrupt.h>
//#include <avr/signal.h>
#include <avr/pgmspace.h>
#include <ctype.h>

#include "uart.h"
#include <util/delay.h>
// #include "sys_config.h"
// #include "timer.h"
// #include "delay.h"

/*
 *  constants and macros
 */

/* size of RX/TX buffers */
#define UART_RX_BUFFER_MASK ( UART_RX_BUFFER_SIZE - 1)
#define UART_TX_BUFFER_MASK ( UART_TX_BUFFER_SIZE - 1)

#if ( UART_RX_BUFFER_SIZE & UART_RX_BUFFER_MASK )
#error RX buffer size is not a power of 2
#endif
#if ( UART_TX_BUFFER_SIZE & UART_TX_BUFFER_MASK )
#error TX buffer size is not a power of 2
#endif

/* ATmega with two USART */
#define ATMEGA_USART0
#define ATMEGA_USART1
#define UART0_RECEIVE_INTERRUPT   USART0_RX_vect
#define UART1_RECEIVE_INTERRUPT   USART1_RX_vect
#define UART0_TRANSMIT_INTERRUPT  USART0_UDRE_vect
#define UART1_TRANSMIT_INTERRUPT  USART1_UDRE_vect

#define UART0_STATUS   UCSR0A
#define UART0_CONTROL  UCSR0B
#define UART0_DATA     UDR0
#define UART0_UDRIE    UDRIE0
#define UART1_STATUS   UCSR1A
#define UART1_CONTROL  UCSR1B
#define UART1_DATA     UDR1
#define UART1_UDRIE    UDRIE1


/*
 *  module global variables
 */
volatile unsigned char setRS485ToReceive;
volatile unsigned char uart_input_timeout_0;
volatile unsigned char uart_output_timeout_0;
volatile unsigned char uart_input_timeout_1;
volatile unsigned char uart_output_timeout_1;


static volatile unsigned char UART_0_TxBuf[UART_TX_BUFFER_SIZE];
static volatile unsigned char UART_0_RxBuf[UART_RX_BUFFER_SIZE];
static volatile unsigned char UART_0_TxHead;
static volatile unsigned char UART_0_TxTail;
static volatile unsigned char UART_0_RxHead;
static volatile unsigned char UART_0_RxTail;
static volatile unsigned char UART_0_LastRxError;

static volatile unsigned char UART_1_TxBuf[UART_TX_BUFFER_SIZE];
static volatile unsigned char UART_1_RxBuf[UART_RX_BUFFER_SIZE];
static volatile unsigned char UART_1_TxHead;
static volatile unsigned char UART_1_TxTail;
static volatile unsigned char UART_1_RxHead;
static volatile unsigned char UART_1_RxTail;
static volatile unsigned char UART_1_LastRxError;



ISR(UART1_RECEIVE_INTERRUPT)
/*************************************************************************
Function: UART Receive Complete interrupt
Purpose:  called when the UART has received a character
**************************************************************************/
{
    unsigned char tmphead;
    unsigned char data;
    unsigned char usr;
    unsigned char lastRxError;


    /* read UART status register and UART data register */
    usr  = UART1_STATUS;
    data = UART1_DATA;

    /* */

    lastRxError = (usr & (_BV(FE1)|_BV(DOR1)) );

    /* calculate buffer index */
    tmphead = ( UART_1_RxHead + 1) & UART_RX_BUFFER_MASK;

    if ( tmphead == UART_1_RxTail ) {
        /* error: receive buffer overflow */
        lastRxError = UART_BUFFER_OVERFLOW >> 8;
    }else{
        /* store new index */
        UART_1_RxHead = tmphead;
        /* store received data in buffer */
        UART_1_RxBuf[tmphead] = data;
    }
    UART_1_LastRxError = lastRxError;
}


ISR(UART1_TRANSMIT_INTERRUPT)
/*************************************************************************
Function: UART Data Register Empty interrupt
Purpose:  called when the UART is ready to transmit the next byte
**************************************************************************/
{
    unsigned char tmptail;

    if ( UART_1_TxHead != UART_1_TxTail) {
        /* calculate and store new buffer index */
        tmptail = (UART_1_TxTail + 1) & UART_TX_BUFFER_MASK;
        UART_1_TxTail = tmptail;
        /* get one byte from buffer and write it to UART */
        UART1_DATA = UART_1_TxBuf[tmptail];  /* start transmission */
    }else{
        /* tx buffer empty, disable UDRE interrupt */
        UART1_CONTROL &= ~_BV(UART1_UDRIE);
        /* set RS 485 xvr to receive */
        //sbi(UCSR1A,6);   //clear TXC by writing one to its bit location!!!
        //not needed
        //UCSR1A |= _BV(6);
        //setRS485ToReceive = 1;
    }
}

#define usart_baudrate2setting(x) \
  (uint16_t)((F_CPU / ((uint32_t)(x) * (uint32_t)16)) - 1)

/*************************************************************************
Function: uart_1_init()
Purpose:  initialize UART1 and set baudrate
Input:    baudrate
Returns:  none
**************************************************************************/
void uart_1_init(unsigned long baudrate, unsigned long xtalCPU){
  unsigned int calcul;

  // UART initialization
  // Communication Parameters: 8 Data, 1 Stop, No Parity
  // UART Receiver: On
  // UART Transmitter: On
  // UART Baud rate: defined by UBRRHI_VALUE & UBRRLO_VALUE above
  calcul = usart_baudrate2setting(baudrate);
  UBRR1H = calcul >> 8;
  UBRR1L = calcul & 0xFF;
  UCSR1B=0x18 | 0xA0 ; // enable data, RX interrupts
  UCSR1C=0x06;

}


/*************************************************************************
Function: uart_1_getc()
Purpose:  return byte from ringbuffer
Returns:  lower byte:  received byte from ringbuffer
          higher byte: last receive error
**************************************************************************/
unsigned int uart_1_getc(void)
{
    unsigned char tmptail;
    unsigned char data;


    if ( UART_1_RxHead == UART_1_RxTail ) {
        return UART_NO_DATA;   /* no data available */
    }

    /* calculate /store buffer index */
    tmptail = (UART_1_RxTail + 1) & UART_RX_BUFFER_MASK;
    UART_1_RxTail = tmptail;

    /* get data from receive buffer */
    data = UART_1_RxBuf[tmptail];

    return (UART_1_LastRxError << 8) + data;

}/* uart_getc */


/*************************************************************************
Function: uart_1_putc()
Purpose:  write byte to ringbuffer for transmitting via UART
Input:    byte to be transmitted
Returns:  none
**************************************************************************/
void uart_1_putc(unsigned char data)
{
    unsigned char tmphead;


    tmphead  = (UART_1_TxHead + 1) & UART_TX_BUFFER_MASK;
 
    while ( tmphead == UART_1_TxTail ){
        ;/* wait for free space in buffer */
    }

    UART_1_TxBuf[tmphead] = data;
    UART_1_TxHead = tmphead;

    /* enable UDRE interrupt */
    UART1_CONTROL    |= _BV(UART1_UDRIE);

}/* uart_putc */


/*************************************************************************
Function: uart_puts()
Purpose:  transmit string to UART
Input:    string to be transmitted
Returns:  none
**************************************************************************/
void uart_1_puts(const char *s )
{
    while (*s)
      uart_1_putc(*s++);

}/* uart_puts */


void uart_1_disable(void){
  UCSR1B = 0x0;
}


void uart_1_send_char(unsigned char ch) //send one character via the serial port
{
  while (!(UCSR1A & 0x20))
    ;    //wait xmit ready
  UDR1 = ch;                 //send the character   			
}

/////////////////////////////////////UART 0///////////////////////////////////////
ISR(UART0_RECEIVE_INTERRUPT)
/*************************************************************************
Function: UART Receive Complete interrupt
Purpose:  called when the UART has received a character
**************************************************************************/
{
    unsigned char tmphead;
    unsigned char data;
    unsigned char usr;
    unsigned char lastRxError;

    /* read UART status register and UART data register */
    usr  = UART0_STATUS;
    data = UART0_DATA;

    /* */

    lastRxError = (usr & (_BV(FE0)|_BV(DOR0)) );

    /* calculate buffer index */
    tmphead = ( UART_0_RxHead + 1) & UART_RX_BUFFER_MASK;

    if ( tmphead == UART_0_RxTail ) {
        /* error: receive buffer overflow */
        lastRxError = UART_BUFFER_OVERFLOW >> 8;
    }else{
        /* store new index */
        UART_0_RxHead = tmphead;
        /* store received data in buffer */
        UART_0_RxBuf[tmphead] = data;
    }
    UART_0_LastRxError = lastRxError;
}


ISR(UART0_TRANSMIT_INTERRUPT)
/*************************************************************************
Function: UART Data Register Empty interrupt
Purpose:  called when the UART is ready to transmit the next byte
**************************************************************************/
{
    unsigned char tmptail;

    if ( UART_0_TxHead != UART_0_TxTail) {
        /* calculate and store new buffer index */
        tmptail = (UART_0_TxTail + 1) & UART_TX_BUFFER_MASK;
        UART_0_TxTail = tmptail;
        /* get one byte from buffer and write it to UART */
        UART0_DATA = UART_0_TxBuf[tmptail];  /* start transmission */
    }else{
        /* tx buffer empty, disable UDRE interrupt */
        UART0_CONTROL &= ~_BV(UART0_UDRIE);
    }
}


/*************************************************************************
Function: uart_0_init()
Purpose:  initialize UART1 and set baudrate
Input:    baudrate
Returns:  none
**************************************************************************/
void uart_0_init(unsigned long baudrate, unsigned long xtalCPU){
  unsigned int calcul;

  // UART initialization
  // Communication Parameters: 8 Data, 1 Stop, No Parity
  // UART Receiver: On
  // UART Transmitter: On
  // UART Baud rate: defined by UBRRHI_VALUE & UBRRLO_VALUE above
  calcul = usart_baudrate2setting(baudrate);
  UBRR0H = calcul >> 8;
  UBRR0L = calcul & 0xFF;
  UCSR0B=0x18 | 0xA0 ; // enable data, RX interrupts
  UCSR0C=0x06;
}


/*************************************************************************
Function: uart_0_getc()
Purpose:  return byte from ringbuffer
Returns:  lower byte:  received byte from ringbuffer
          higher byte: last receive error
**************************************************************************/
unsigned int uart_0_getc(void)
{
    unsigned char tmptail;
    unsigned char data;


    if ( UART_0_RxHead == UART_0_RxTail ) {
        return UART_NO_DATA;   /* no data available */
    }

    /* calculate /store buffer index */
    tmptail = (UART_0_RxTail + 1) & UART_RX_BUFFER_MASK;
    UART_0_RxTail = tmptail;

    /* get data from receive buffer */
    data = UART_0_RxBuf[tmptail];

    return (UART_0_LastRxError << 8) + data;

}/* uart_getc */


/*************************************************************************
Function: uart_0_putc()
Purpose:  write byte to ringbuffer for transmitting via UART
Input:    byte to be transmitted
Returns:  none
**************************************************************************/
void uart_0_putc(unsigned char data)
{
    unsigned char tmphead;


    tmphead  = (UART_0_TxHead + 1) & UART_TX_BUFFER_MASK;

    while ( tmphead == UART_0_TxTail ){
        ;/* wait for free space in buffer */
    }

    UART_0_TxBuf[tmphead] = data;
    UART_0_TxHead = tmphead;

    /* enable UDRE interrupt */
    UART0_CONTROL    |= _BV(UART0_UDRIE);

}/* uart_putc */


/*************************************************************************
Function: uart_puts()
Purpose:  transmit string to UART
Input:    string to be transmitted
Returns:  none
**************************************************************************/
void uart_0_puts(const char *s )
{
    while (*s)
      uart_0_putc(*s++);

}/* uart_puts */


void uart_0_disable(void){
  UCSR0B = 0x0;
}


void uart_0_send_char(unsigned char ch) //send one character via the serial port
{
  while (!(UCSR0A & 0x20))
    ;    //wait xmit ready
  UDR0 = ch;                 //send the character   			
}










