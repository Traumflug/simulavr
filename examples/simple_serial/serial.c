/*
 *  $Id$
 *
 * Serial subsystem
 *
 * Copyright (C) 2009 Michael Moon <triffid.hunter@gmail.com>
 *               2010 Markus Hitter <mah@jump-ing.de>
 *
 * This is a pretty lean, fast and powerful serial subsystem, taken from
 * RepRaps' Teacup firmware: https://github.com/Traumflug/Teacup_Firmware .
 * For even more simplification, support for XON/XOFF flow control was stripped.
 *
 * The system uses ringbuffers for both transmit and receive, and
 * intelligently decides whether to wait or drop transmitted characters if
 * the buffer is full. It's tested on many versions of the ATmega.
 */
#include "serial.h"

#include <avr/interrupt.h>

// Baud rate.
#define BAUD 19200

// Size of TX and RX buffers. MUST be a power of 2 and 256 or less.
#define BUFSIZE 64

// rx buffer head pointer. Points to next available space.
volatile uint8_t rxhead = 0;
// rx buffer tail pointer. Points to last character in buffer.
volatile uint8_t rxtail = 0;
// rx buffer
volatile uint8_t rxbuf[BUFSIZE];

// tx buffer head pointer. Points to next available space.
volatile uint8_t txhead = 0;
// tx buffer tail pointer. Points to last character in buffer.
volatile uint8_t txtail = 0;
// tx buffer
volatile uint8_t txbuf[BUFSIZE];

// check if we can read from this buffer
#define buf_canread(buffer) ((buffer ## head - buffer ## tail) & (BUFSIZE - 1))
// read from buffer
#define buf_pop(buffer, data) do { data = buffer ## buf[buffer ## tail]; buffer ## tail = (buffer ## tail + 1) & (BUFSIZE - 1); } while (0)

// check if we can write to this buffer
#define buf_canwrite(buffer) ((buffer ## tail - buffer ## head - 1) & (BUFSIZE - 1))
// write to buffer
#define buf_push(buffer, data) do { buffer ## buf[buffer ## head] = data; buffer ## head = (buffer ## head + 1) & (BUFSIZE - 1); } while (0)

/*
 * ringbuffer logic:
 * head = written data pointer
 * tail = read data pointer
 *
 * when head == tail, buffer is empty
 * when head + 1 == tail, buffer is full
 * thus, number of available spaces in buffer is (tail - head) & bufsize
 *
 * can write:
 * (tail - head - 1) & (BUFSIZE - 1)
 *
 * write to buffer:
 * buf[head++] = data; head &= (BUFSIZE - 1);
 *
 * can read:
 * (head - tail) & (BUFSIZE - 1)
 *
 * read from buffer:
 * data = buf[tail++]; tail &= (BUFSIZE - 1);
 */

// initialise serial subsystem
void serial_init() {
  // set up baud generator and interrupts, clear buffers

  #if BAUD > 38401
    UCSR0A = MASK(U2X0);
    UBRR0 = (((F_CPU / 8) / BAUD) - 0.5);
  #else
    UCSR0A = 0;
    UBRR0 = (((F_CPU / 16) / BAUD) - 0.5);
  #endif

  UCSR0B = MASK(RXEN0) | MASK(TXEN0);
  UCSR0C = MASK(UCSZ01) | MASK(UCSZ00);

  UCSR0B |= MASK(RXCIE0) | MASK(UDRIE0);
}

// receive interrupt
// Using the pragma inside the function is incompatible with Arduinos' gcc.
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#ifdef  USART_RX_vect
ISR(USART_RX_vect) {
#else
ISR(USART0_RX_vect) {
#endif
  // we have received a character, stuff it in the rx buffer
  // if we can, or drop it if we can't
  if (buf_canwrite(rx))
    buf_push(rx, UDR0);
  else {
    // Not reading the character makes the interrupt logic to swamp us with
    // retries, so better read it and throw it away.
    // #pragma GCC diagnostic ignored "-Wunused-but-set-variable"
    uint8_t trash;
    // #pragma GCC diagnostic pop

    trash = UDR0;
  }
}
#pragma GCC diagnostic pop

// transmit buffer ready interrupt
#ifdef  USART_UDRE_vect
ISR(USART_UDRE_vect) {
#else
ISR(USART0_UDRE_vect) {
#endif
  // provide the next character to transmit if we can,
  // otherwise disable this interrupt
  if (buf_canread(tx))
    buf_pop(tx, UDR0);
  else
    UCSR0B &= ~MASK(UDRIE0);
}

// check how many characters can be read
uint8_t serial_rxchars() {
  return buf_canread(rx);
}

// read one character
uint8_t serial_popchar() {
  uint8_t c = 0;

  // it's imperative that we check, because if the buffer is empty
  // and we pop, we'll go through the whole buffer again
  if (buf_canread(rx))
    buf_pop(rx, c);

  return c;
}

// send one character
void serial_writechar(uint8_t data) {
  // check if interrupts are enabled
  if (SREG & MASK(SREG_I)) {
    // if they are, we should be ok to block since the
    // tx buffer is emptied from an interrupt
    for ( ; buf_canwrite(tx) == 0; );
    buf_push(tx, data);
  }
  else {
    // interrupts are disabled- maybe we're in one?
    // anyway, instead of blocking, only write if we have room
    if (buf_canwrite(tx))
      buf_push(tx, data);
  }
  // enable TX interrupt so we can send this character
  UCSR0B |= MASK(UDRIE0);
}

// send a string
void serial_writestr(uint8_t *data) {
  uint8_t i = 0, r;

  // Yes, this is *supposed* to be assignment rather than comparison, so we
  // break when r is assigned zero.
  while ((r = data[i++]))
    serial_writechar(r);
}

/*
 * Write from FLASH
 *
 * Extensions to output flash memory pointers. This prevents the data to
 * become part of the .data segment instead of the .code segment. That means
 * less memory is consumed for multi-character writes.
 *
 * For single character writes (i.e. '\n' instead of "\n"), using
 * serial_writechar() directly is the better choice.
*/
void serial_writestr_P(PGM_P data) {
  uint8_t r, i = 0;

  // Yes, this is *supposed* to be assignment rather than comparison, so we
  // break when r is assigned zero.
  while ((r = pgm_read_byte(&data[i++])))
    serial_writechar(r);
}

