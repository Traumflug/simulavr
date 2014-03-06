/*
 ****************************************************************************
 *
 * simulavr - A simulator for the Atmel AVR family of microcontrollers.
 * Copyright (C) 2001, 2002, 2003   Klaus Rudolph		
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
 */

#ifndef HARDWARE
#define HARDWARE

class AvrDevice;

/*! Hardware objects are the subsystems of an AVR device. They have a clock and
  reset input and in addition will define various memory registers through
  which they can be accessed. */
class Hardware {
    
    public:
        /*! Creates new Hardware and makes it part of the given AvrDevice hw. The
          hardware will receive all reset signals from the AVR, but must
          register clock cycling needs itself. */
        Hardware(AvrDevice *core);
        virtual ~Hardware() {};

        /*! Called for each AVR cycle when this hardware has registered itself
          as a receiver for AVR clocks. Returns nonzero if instructions should
          not be executed (e.g. a Flash write is in progress). */
        virtual unsigned int CpuCycle(void) { return 0; }

        /*! Implement the hardware's reset functionality here. The default
          is no action on reset. */
        virtual void Reset(void) {};

        /*! This signals the hardware that the given IRQ vector has been handled
          by the AVR core. */
        virtual void ClearIrqFlag(unsigned int vector) {}
        
        /*! Does the hardware have a level interrupt (triggered otherwise, the default!) */
        virtual bool IsLevelInterrupt(unsigned int vector) { return false; }
        
        /*! Check a level interrupt on the time, where interrupt routine will be called */
        virtual bool LevelInterruptPending(unsigned int vector) { return false; }
        
};

#endif
