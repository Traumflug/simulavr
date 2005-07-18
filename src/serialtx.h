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
 */
#ifndef SERIALTX
#define SERIALTX

#include "systemclocktypes.h"
#include "ui.h"

class SerialTxBuffered: public SimulationMember {
    protected:
        Pin tx;

        map < string, Pin *> allPins;
        unsigned long long baudrate;

        enum T_TxState{
            TX_DISABLED,
            TX_SEND_STARTBIT,
            TX_SEND_DATABIT,
            TX_SEND_STOPBIT,
            TX_STOPPING
        } ;

        T_TxState txState;

        vector<unsigned char> inputBuffer;
        unsigned int data;
        unsigned int bitCnt;
        unsigned int maxBitCnt;
       

    public:
        SerialTxBuffered();
        void Reset();
        virtual ~SerialTxBuffered(){};
        virtual int Step(bool &trueHwStep, SystemClockOffset *timeToNextStepIn_ns=0);
        virtual void Send(unsigned char data);
        virtual void SetBaudRate(SystemClockOffset baud);
        Pin* GetPin(const char *name); 
};


class SerialTx: public SerialTxBuffered, public ExternalType {
    protected:
        UserInterface *ui;
        string name;

        unsigned int CpuCycleTx();

    public:
        SerialTx(UserInterface *_ui, const char *_name, const char *baseWindow);
        unsigned int CpuCycle();
        virtual ~SerialTx(){};
        virtual void SetNewValueFromUi(const string &);
 };

#endif
