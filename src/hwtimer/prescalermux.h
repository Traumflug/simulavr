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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 ****************************************************************************
 *
 *  $Id$
 */

#ifndef PRESCALERMUX
#define PRESCALERMUX

#include "timerprescaler.h"
#include "pinatport.h"

//! PrescalerMultiplexer without external count pin
/*! Multiplexer with the following clock rates: no clock, CK, CK/8, CK/32,
  CK/64, CK/128, CK/256, CK/1024. Clock is the clock from prescaler. */
class PrescalerMultiplexer {
  
    protected:
        HWPrescaler* prescaler; //!< pointer to prescaler
  
    public:
        //! Creates a multiplexer instance, connected with prescaler
        PrescalerMultiplexer(HWPrescaler *ps);
        //! Requests a clock event depending on cs
        //! @param cs multiplexer select value
        //! @return true, if a clock event occured
        virtual bool isClock(unsigned int cs);
    
};

//! PrescalerMultiplexer with external count pin
/*! Multiplexer with the following clock rates: no clock, CK, CK/8,
  CK/64, CK/256, CK/1024 and falling or rising edge on external count pin.
  Clock is the clock from prescaler. */
class PrescalerMultiplexerExt: public PrescalerMultiplexer {
  
    protected:
        PinAtPort clkpin;
        bool      clkpin_old;
    
    public:
        //! Creates a multiplexer instance with a count input pin, connected with prescaler
        PrescalerMultiplexerExt(HWPrescaler *ps, PinAtPort pi);
        virtual bool isClock(unsigned int cs);
    
};

//! PrescalerMultiplexer for ATTiny15
/*! \todo for the moment it's a placeholder, no multiplexing functionality */
class PrescalerMultiplexerT15: public PrescalerMultiplexer {
  
    public:
        //! Creates a multiplexer instance for timer 1 on ATTiny15, connected with prescaler
        PrescalerMultiplexerT15(HWPrescaler *ps);
        virtual bool isClock(unsigned int cs);
    
};

#endif // PRESCALERMUX
