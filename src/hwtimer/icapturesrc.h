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

#ifndef ICAPTURESRC
#define ICAPTURESRC

#include "../pinatport.h"

class HWAcomp;

//! Class, which provides input capture source for 16bit timers
class ICaptureSource {
    
    protected:
        PinAtPort capturePin;
        HWAcomp *acomp;
        bool acic;
  
    public:
        ICaptureSource(PinAtPort cp);
        virtual ~ICaptureSource() { }
        
        //! Returns the digital input state of input capture source(s)
        virtual bool GetSourceState(void);

        //! Register analog comparator
        void RegisterAComp(HWAcomp *_acomp) { acomp = _acomp; }

        //! Reflect ACIC flag state
        void SetACIC(bool _acic) { acic = _acic; }
};

#endif
