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

#ifndef KEYBDOARD_H_INCLUDED
#define KEYBDOARD_H_INCLUDED

#define KBD_BUFFER_SIZE 128


#include <iostream>

#include "systemclocktypes.h"
#include "simulationmember.h"
#include "hardware.h"
#include "pin.h"


/** A PS2-style keyboard sending scan-codes obtained from UI to pins on device (I guess). */
class Keyboard : public SimulationMember, public ExternalType {
    protected:
        unsigned char myPortValue;
        Pin clk;
        Pin data;

        unsigned int bitCnt;

        //ofstream debugOut;
        SystemClockOffset myClockFreq;

        unsigned int buffer[KBD_BUFFER_SIZE];
        unsigned int bufferWriteIndex;
        unsigned int bufferReadIndex;

        void InsertMakeCodeToBuffer(int);
        void InsertBreakCodeToBuffer(int);
        int InsertScanCodeToBuffer( unsigned char scan);

        unsigned char actualChar;
        unsigned char lastPortValue;

    public:
        void SetNewValueFromUi(const std::string &);
        virtual int Step(bool &trueHwStep, SystemClockOffset *timeToNextStepIn_ns=0);
        Keyboard(UserInterface *, const char *name, const char *baseWindow);
        void SetClockFreq(SystemClockOffset f);
        virtual ~Keyboard();
};

#endif
