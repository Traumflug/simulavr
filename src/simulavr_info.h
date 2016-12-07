/*
 ****************************************************************************
 *
 * simulavr - A simulator for the Atmel AVR family of microcontrollers.
 * Copyright (C) 2013 Markus Hitter <mah@jump-ing.de>
 * ELF storage strategy inspired by simavr by Michel Pollet.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 ****************************************************************************
 *
 *   $Id$
 *
 * This header provides macros to embed simulator setup information into
 * a compiled ELF binary.
 *
 * Example:
 *
 * Add this somewhere at the root level of your AVR code:
 *
 *   #include "simulavr_info.h"
 *   SIMINFO_DEVICE("atmega644");
 *   SIMINFO_CPUFREQUENCY(F_CPU);
 *
 * Then link as usual, but add these linker flags to avr-gcc to prohibit
 * the linker from removing the info sections at the link stage:
 *
 *   -Wl,--section-start=.siminfo=0x900000
 *   -u siminfo_device
 *   -u siminfo_cpufrequency
 *   -u siminfo_serial_in
 *   -u siminfo_serial_out
 *
 * The value choosen here to be 0x900000 can be choosen freely, but must
 * be above 0x840400, else it can conflict with program / eeprom / fuses /
 * lockbits / signature data, see ELFLoad() in src/avrreadelf.cpp, line 215ff.
 *
 * Having this done, running the ELF binary in the simulator will
 * automatically inform simulavr for which AVR variant and CPU frequency
 * the binary was built, making the corresponding command line parameters
 * obsolete. In case you give both, in-binary and CLI parameters, CLI
 * parameters take precedence.
 *
 * The really nice thing about this mechanism is, it doesn't alter the
 * executed binary at all. You can upload and run this on real hardware,
 * not a single byte of Flash memory or a single CPU cycle at runtime wasted.
 */

#ifndef __SIMULAVR_INFO_H__
#define __SIMULAVR_INFO_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


#define SIMINFO_SECTION __attribute__((section(".siminfo")))

enum {
  SIMINFO_TAG_NOTAG = 0, // keep this unused as a protection against empty data
  SIMINFO_TAG_DEVICE,
  SIMINFO_TAG_CPUFREQUENCY,
  SIMINFO_TAG_SERIAL_OUT,
  SIMINFO_TAG_SERIAL_IN,
};

struct siminfo_long_t {
  uint8_t tag;
  uint8_t length;
  uint32_t value;
} __attribute__((__packed__));

struct siminfo_string_t {
  uint8_t tag;
  uint8_t length;
  char string[];
} __attribute__((__packed__));

struct siminfo_serial_t {
  uint8_t tag;
  uint8_t length;
  char pin[3];
  uint32_t baudrate;
  char filename[];
} __attribute__((__packed__));


/*
 * This gives the device type, like "attiny45", "atmega128", "atmega644", etc.
 */
#define SIMINFO_DEVICE(name) \
  const struct siminfo_string_t siminfo_device SIMINFO_SECTION = { \
    SIMINFO_TAG_DEVICE, \
    /* We could use sizeof(siminfo_device) here, but avr-gcc has \
       been seen to set length to 0 (zero), then. */ \
    sizeof(name) + 2, \
    name \
  }

/*
 * This gives the cpu frequency, like 8000000UL or 16000000UL.
 */
#define SIMINFO_CPUFREQUENCY(value) \
  const struct siminfo_long_t siminfo_cpufrequency SIMINFO_SECTION = { \
    SIMINFO_TAG_CPUFREQUENCY, \
    sizeof(uint32_t) + 2, \
    value \
  }

/*
 * Create a serial in (Rx, to AVR) component. The the sent characters/bytes
 * will be taken from the given file. This component can be connected to the
 * same file as a serial out, if it's a special file like a real serial device
 * or a pipe. Connecting both to the same regular file will mess things up.
 *
 * Using "-" as file name means connecting to the console (stdin/stdout).
 *
 * The pin to connect is named by a 2-character string, where "E2" means
 * pin 2 on port E.
 *
 * Why a baud rate? Well, the component doesn't just write to the UART receive
 * register, but synthesizes actual serial signals on the pin, which in turn
 * should be interpreted by your AVR code. If your code sets a baud rate not
 * matching the one given here, serial communications won't work. Just like a
 * real serial device configured to work at 19200 baud won't work on a real
 * serial port set to something else.
 *
 * Other parameters are fixed to 8N1, which means 8 bits, no parity, 1 stop bit.
 */
#define SIMINFO_SERIAL_IN(pin, filename, baudrate) \
  const struct siminfo_serial_t siminfo_serial_in SIMINFO_SECTION = { \
    SIMINFO_TAG_SERIAL_IN, \
    sizeof(char[3]) + sizeof(uint32_t) + sizeof(filename) + 2, \
    pin, \
    baudrate, \
    filename \
  }

/*
 * Create a serial out (Tx, from AVR) component. Same as above, but the
 * other direction. The serial port pin is continuously read and interpreted.
 */
#define SIMINFO_SERIAL_OUT(pin, filename, baudrate) \
  const struct siminfo_serial_t siminfo_serial_out SIMINFO_SECTION = { \
    SIMINFO_TAG_SERIAL_OUT, \
    sizeof(char[3]) + sizeof(uint32_t) + sizeof(filename) + 2, \
    pin, \
    baudrate, \
    filename \
  }


#ifdef __cplusplus
};
#endif

#endif /* __SIMULAVR_INFO_H__ */
