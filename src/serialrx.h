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

#ifndef SERIALRX
#define SERIALRX

#include "systemclocktypes.h"
#include "ui.h"
#include "pinnotify.h"


class SerialRxBasic: public SimulationMember, public HasPinNotifyFunction {
    protected:
        Pin rx;
        map < string, Pin *> allPins;
        unsigned long long baudrate;

        void PinStateHasChanged(Pin*);
        virtual void CharReceived(unsigned char c)=0;

        int highCnt;

        int bitCnt;
        int maxBitCnt;
        int dataByte;

        enum T_RxState {
            RX_WAIT_LOWEDGE,
            RX_READ_STARTBIT,
            RX_READ_DATABIT_START,
            RX_READ_DATABIT_FIRST,
            RX_READ_DATABIT_SECOND,
            RX_READ_DATABIT_THIRD,
        } ;

        T_RxState rxState;

        bool sendInHex;

    public:
    	void SetBaudRate(SystemClockOffset baud);
    	void SetHexOutput(bool newValue);
        SerialRxBasic();
        void Reset();
        virtual Pin* GetPin(const char *name) ;
        virtual ~SerialRxBasic(){};
        virtual int Step(bool &trueHwStep, SystemClockOffset *timeToNextStepIn_ns=0);
 };
 
 
 // ===========================================================================
 
 
 class SerialRxBuffered: public SerialRxBasic{
 	protected:
 		vector<unsigned char> buffer;
 		virtual void CharReceived(unsigned char c);
 	public:
 		unsigned char Get();
 		long Size();
 };


// ===========================================================================


class SerialRx: public SerialRxBasic, public ExternalType{
    protected:
        UserInterface *ui;
        string name;

        map < string, Pin *> allPins;

        unsigned int CpuCycleRx();
        virtual void CharReceived(unsigned char c);

    public:
        SerialRx(UserInterface *_ui, const char *_name, const char *baseWindow);
        unsigned int CpuCycle();
        virtual ~SerialRx(){};
        virtual void SetNewValueFromUi(const string &);
 };

#endif
