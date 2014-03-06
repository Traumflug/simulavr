/*
 ****************************************************************************
 *
 * simulavr - A simulator for the Atmel AVR family of microcontrollers.
 * Copyright (C) 2001, 2002, 2003, 2004   Klaus Rudolph
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

#include "serialtx.h"
#include "systemclock.h"
#include "string2.h"
#include "avrerror.h"

using namespace std;

SerialTxBuffered::SerialTxBuffered()
{
    allPins["tx"] = &tx;
    Reset();
}

void SerialTxBuffered::Reset()
{
    txState=TX_DISABLED;
    receiveInHex=false;
    baudrate=115200;
    maxBitCnt=8;
    tx='H'; 
}

Pin* SerialTxBuffered::GetPin(const char* name)
{
    return allPins[name];
}

int SerialTxBuffered::Step(
        bool &trueHwStep, SystemClockOffset *timeToNextStepIn_ns)
{
    switch (txState) {
        case TX_SEND_STARTBIT:
            data=*(inputBuffer.begin());
            inputBuffer.erase(inputBuffer.begin());
            tx='L';
            bitCnt=0;
            //all we measures are in ns !;
            *timeToNextStepIn_ns=(SystemClockOffset)1e9/baudrate;
            txState=TX_SEND_DATABIT;
            break;

        case TX_SEND_DATABIT:
            if (( data >> bitCnt ) & 0x01)  {
                tx='H';
            } else {
                tx='L';
            }
            //all we measures are in ns !;
            *timeToNextStepIn_ns=(SystemClockOffset)1e9/baudrate;
            bitCnt++;
            if(bitCnt>=maxBitCnt) txState=TX_SEND_STOPBIT;
            break;

        case TX_SEND_STOPBIT:
            tx='H';
            txState=TX_STOPPING;
            //all we measures are in ns !;
            *timeToNextStepIn_ns=(SystemClockOffset)1e9/baudrate;
            break;

        case TX_STOPPING:
            if (inputBuffer.size()>0) { //another char to send!
                txState=TX_SEND_STARTBIT;
                *timeToNextStepIn_ns=0;//(SystemClockOffset)1e9/baudrate;
            } else {
                txState=TX_DISABLED;
                *timeToNextStepIn_ns=-1; //last step, do not continue
            }

            break;

        default:
            avr_error("Illegal state in SerialTx");
    }


    return 0;

}

void SerialTxBuffered::Send(unsigned char data)
{
    inputBuffer.push_back(data); //write new char to input buffer

    cerr << "TX: " << hex << data << " ";
    //if we not active, activate tx machine now
    if (txState==TX_DISABLED) {
        txState=TX_SEND_STARTBIT;
        SystemClock::Instance().Add(this);
    }
}

void SerialTxBuffered::SetBaudRate(SystemClockOffset baud){
    baudrate = baud;
}

void SerialTxBuffered::SetHexInput(bool newValue){
    receiveInHex = newValue;
}



// ===========================================================================
// ===========================================================================
// ===========================================================================


SerialTx::SerialTx(UserInterface *ui, const char *name, const char *baseWindow)
{
    ostringstream os;
    os << "create SerialTx " << name  << " " << baseWindow << endl;
    ui->Write(os.str());
    ui->AddExternalType(name, this);
    Reset();
}

void SerialTx::SetNewValueFromUi(const string &s) {
    cout << "SerialTx::SetNewValueFromUi >" << s << "<" << endl;
    if ( receiveInHex ) {
        unsigned char value;
        bool          rc;
        rc = StringToUnsignedChar( s.c_str(), &value, NULL, 16 );
        if ( !rc ) {
            cerr << "SerialTx::SetNewValueFromUi:: bad conversion" << endl;
        } else {
            // cerr << "SerialTx::Send " << hex << (unsigned int) value << endl;
            Send(value);
        }
    } else {
        if (s == "__SPACE__")  { Send(' '); }
        else 
        {
            for(unsigned int i=0; i < s.length(); i++)
            {
                Send(s[i]);
            }
        }
    }
}
