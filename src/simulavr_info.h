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
 *
 * The value choosen here to be 0x900000 can be choosen freely, but must
 * be above 0x840400, else it can conflict with program / eeprom / fuses /
 * lockbits / signature data, see ELFLoad() in src/avrreadelf.cpp, line 215ff.
 *
 * You also have to avoid the -Wl,--gc-sections flag, which unfortunately
 * increases binary size if you have unused functions.
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


#ifdef __cplusplus
};
#endif

#endif /* __SIMULAVR_INFO_H__ */
