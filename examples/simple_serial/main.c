 /*
 ****************************************************************************
 *
 * simple_serial - A demo for the SimulAVR simulator.
 * Copyright (C) 2013 Markus Hitter <mah@jump-ing.de>
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
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 ****************************************************************************
 *
 *  $Id$
 *
 * Super trivial example exercising the UART serial communications line.
 *
 * It's purpose is to show how SimulAVR can redirect serial communications
 * in a way useful for running in a simulator, while requiring NO code
 * modifications which would change it's behaviour compared to running
 * on real hardware or disallowing to run the very same compiled binary
 * on that hardware. The compiled binary should work in the simulator just
 * as fine as on hardware.
 */

#include "serial.h"

#include <avr/interrupt.h>

/*
 * Here we tell SimulAVR for which device and which frequency we build this
 * code. The nice thing is, these macros add a section to the ELF file, so
 * they can be read by SimulAVR, but they're also ignored when converted to
 * an ihex file for hardware upload. Accordingly, the executed binary for
 * SimulAVR and real hardware is the same, no need to recompile for one or
 * another. The more important result of this is, behaviour is also exactly
 * the same, no extra cycles for serving simulator matters.
 *
 * One small caveat: To stop the linker from stripping this section, avr-gcc
 * needs an extra parameter when linking the binary (see Makefile.am):
 *
 *   -Wl,--section-start=.siminfo=0x900000
 *
 * For details on these macros, see the comments in simulavr_info.h.
 */
#include "../../src/simulavr_info.h"
SIMINFO_DEVICE("atmega644");
SIMINFO_CPUFREQUENCY(F_CPU);
SIMINFO_SERIAL_OUT("D1", "-", 19200);


// This is all we need:
int main (void) {

  serial_init();
  sei();

  serial_writestr_P(PSTR("Hello, world!\n\nNow, please type:\n"));

  for (;;) {
    if (serial_rxchars() != 0) {
      uint8_t c = serial_popchar();
      serial_writestr_P(PSTR("received: <"));
      serial_writechar(c);
      serial_writestr_P(PSTR(">\n"));
    }
  }
}
