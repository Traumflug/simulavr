#ifndef UART_H
#define UART_H

/**
 *  @defgroup pfleury_uart UART Library
 *  @code #include <uart.h> @endcode
 *
 *  @brief Interrupt UART library using the built-in UART with transmit and receive circular buffers.
 *
 *  This library can be used to transmit and receive data through the built in UART.
 *
 *  An interrupt is generated when the UART has finished transmitting or
 *  receiving a byte. The interrupt handling routines use circular buffers
 *  for buffering received and transmitted data.
 *
 *  The UART_RX_BUFFER_SIZE and UART_TX_BUFFER_SIZE constants define
 *  the size of the circular buffers in bytes. Note that these constants must be a power of 2.
 *  You may need to adapt this size to your target and your application.
 *
 *  @note Based on Atmel Application Note AVR306
 *  @author Peter Fleury pfleury@gmx.ch  http://jump.to/fleury,
 */

/*@{*/

#if (__GNUC__ * 100 + __GNUC_MINOR__) < 303
#error "This library requires AVR-GCC 3.3 or later, update to newer AVR-GCC compiler !"
#endif

extern volatile unsigned char uart_input_timeout_0;
extern volatile unsigned char uart_output_timeout_0;
extern volatile unsigned char uart_input_timeout_1;
extern volatile unsigned char uart_output_timeout_1;

/*
** constants and macros
*/

/** @brief  UART Baudrate Expression
 *  @param  xtalcpu  system clock in Mhz
 *  @param  baudrate baudrate in bps, e.g. 1200, 2400, 9600
 */
#define UART_BAUD_SELECT(baudRate,xtalCpu) ((xtalCpu)/((baudRate)*16l)-1)


/** Size of the circular receive buffer, must be power of 2 */
#define UART_RX_BUFFER_SIZE 256
/** Size of the circular transmit buffer, must be power of 2 */
//#define UART_TX_BUFFER_SIZE 64
//increase size for calibration data to go out
#define UART_TX_BUFFER_SIZE 256



#ifndef P
#define P(s) ({static const char c[] __attribute__ ((progmem)) = s;c;})
#endif


/*
** high byte error return code of uart_getc()
*/
#define UART_FRAME_ERROR      0x0800              /* Framing Error by UART       */
#define UART_OVERRUN_ERROR    0x0400              /* Overrun condition by UART   */
#define UART_BUFFER_OVERFLOW  0x0200              /* receive ringbuffer overflow */
#define UART_NO_DATA          0x0100              /* no receive data available   */


/*
** function prototypes
*/

/**
   @brief   Initialize UART and set baudrate
   @param   baudrate Specify baudrate using macro UART_BAUD_SELECT()
   @return  none
*/
extern void uart_1_init(unsigned long baudrate, unsigned long xtalCPU);
extern void uart_0_init(unsigned long baudrate, unsigned long xtalCPU);


/**
 *  @brief   Get received byte from ringbuffer
 *
 * Returns in the lower byte the received character and in the
 * higher byte the last receive error.
 * UART_NO_DATA is returned when no data is available.
 *
 *  @param   void
 *  @return  lower byte:  received byte from ringbuffer
 *  @return  higher byte: last receive status
 *           - \b 0 successfully received data from UART
 *           - \b UART_NO_DATA
 *             <br>no receive data available
 *           - \b UART_BUFFER_OVERFLOW
 *             <br>Receive ringbuffer overflow.
 *             We are not reading the receive buffer fast enough,
 *             one or more received character have been dropped
 *           - \b UART_OVERRUN_ERROR
 *             <br>Overrun condition by UART.
 *             A character already present in the UART UDR register was
 *             not read by the interrupt handler before the next character arrived,
 *             one or more received characters have been dropped.
 *           - \b UART_FRAME_ERROR
 *             <br>Framing Error by UART
 */
extern unsigned int uart_0_getc(void);
extern unsigned int uart_1_getc(void);


/**
 *  @brief   Put byte to ringbuffer for transmitting via UART
 *  @param   data byte to be transmitted
 *  @return  none
 */
extern void uart_0_putc(unsigned char data);
extern void uart_1_putc(unsigned char data);


/**
 *  @brief   Put string to ringbuffer for transmitting via UART
 *
 *  The string is buffered by the uart library in a circular buffer
 *  and one character at a time is transmitted to the UART using interrupts.
 *  Blocks if it can not write the whole string into the circular buffer.
 *
 *  @param   s string to be transmitted
 *  @return  none
 */
extern void uart_0_puts(const char *s );
extern void uart_1_puts(const char *s );

extern void uart_0_disable(void);
extern void uart_1_disable(void);


extern void uart_0_send_char(unsigned char ch);
extern void uart_1_send_char(unsigned char ch);


/*@}*/
#endif // UART_H








