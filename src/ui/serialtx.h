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

#ifndef SERIALTX_H_INCLUDED
#define SERIALTX_H_INCLUDED

#include "systemclocktypes.h"
#include "ui.h"

class SerialTxBuffered: public SimulationMember {
    protected:
        Pin tx;

        std::map < std::string, Pin *> allPins;
        unsigned long long baudrate;

        enum T_TxState{
            TX_DISABLED,
            TX_SEND_STARTBIT,
            TX_SEND_DATABIT,
            TX_SEND_STOPBIT,
            TX_STOPPING
        } ;

        T_TxState txState;

        std::vector<unsigned char> inputBuffer;
        unsigned int data;
        unsigned int bitCnt;
        unsigned int maxBitCnt;
       
        bool receiveInHex;

    public:
        SerialTxBuffered();
        void Reset();
        virtual ~SerialTxBuffered(){};
        void SetHexInput(bool newValue);
        virtual int Step(bool &trueHwStep, SystemClockOffset *timeToNextStepIn_ns=0);
        /// Add byte from UI to be sent to device's UART.
        virtual void Send(unsigned char data);
        bool Sending(void);
        virtual void SetBaudRate(SystemClockOffset baud);
        virtual Pin* GetPin(const char *name); 
};

#include <stdio.h>
#include <unistd.h>

/** Reads  bytes from a file, a special file/com port or the console
 *  and sends them to the device's UART. */
class SerialTxFile: public SerialTxBuffered {
    private:
        // Use a classic file descriptor to have better
        // control over buffers (and their avoidance).
        int fd;
    public:
        SerialTxFile(const char *filename);
        ~SerialTxFile();
        virtual int Step(bool &trueHwStep, SystemClockOffset *timeToNextStepIn_ns=0);
};


/** Buffers byte from UI to be sent to device's UART. */
class SerialTx: public SerialTxBuffered, public ExternalType {
    public:
        SerialTx(UserInterface *_ui, const char *_name, const char *baseWindow);
        virtual ~SerialTx(){};
        virtual void SetNewValueFromUi(const std::string &);
 };

#endif
