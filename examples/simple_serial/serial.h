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

#ifndef _SERIAL_H
#define _SERIAL_H

#include <stdint.h>
#include <avr/io.h>
#include <avr/pgmspace.h>

#define MASK(PIN) (1 << PIN)

// initialise serial subsystem
void serial_init(void);

// return number of characters in the receive buffer,
// and number of spaces in the send buffer
uint8_t serial_rxchars(void);

// read one character
uint8_t serial_popchar(void);

// send one character
void serial_writechar(uint8_t data);

// read/write many characters
void serial_writestr(uint8_t *data);

// write from flash
void serial_writestr_P(PGM_P data);

#endif  /* _SERIAL_H */

