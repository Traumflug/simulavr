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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 ****************************************************************************
 *
 *  $Id$
 */

#include "serialrx.h"
#include "systemclock.h"
#include "systemclocktypes.h"

using namespace std;

SerialRxBasic::SerialRxBasic(){
    rx.RegisterCallback(this);
    allPins["rx"]= &rx;
    sendInHex = false;
    Reset();
}

void SerialRxBasic::PinStateHasChanged(Pin* p){
    if (!*p) { //Low
        if (rxState== RX_WAIT_LOWEDGE) {
            rxState=RX_READ_STARTBIT;
            SystemClock::Instance().Add(this); //as next Step() is called
        }
    }
}

void SerialRxBasic::Reset(){
    baudrate=115200;
    maxBitCnt=10; //Start+8Data+Stop
    rxState=RX_WAIT_LOWEDGE;
}

Pin* SerialRxBasic::GetPin(const char* name){
    return allPins[name];
}

int SerialRxBasic::Step(bool &trueHwStep, SystemClockOffset *timeToNextStepIn_ns){
    switch (rxState) {
        case RX_READ_STARTBIT: //wait until first edge of databit
            *timeToNextStepIn_ns= (SystemClockOffset)1e9/baudrate/16*7;
            rxState=RX_READ_DATABIT_FIRST;
            dataByte=0;
            bitCnt=0;
            break;

        case RX_READ_DATABIT_FIRST:   //(1/7)
            *timeToNextStepIn_ns= (SystemClockOffset)1e9/baudrate/16;
            rxState= RX_READ_DATABIT_SECOND;
            if (rx) {
                highCnt++;
            }
            break;

        case RX_READ_DATABIT_SECOND: //(1/8)
            *timeToNextStepIn_ns= (SystemClockOffset)1e9/baudrate/16;
            rxState= RX_READ_DATABIT_THIRD;
            if (rx) {
                highCnt++;
            }

            break;

        case RX_READ_DATABIT_THIRD: //(1/9)
            rxState= RX_READ_DATABIT_FIRST;
            if (rx) {
                highCnt++;
            }

            if (highCnt>1) {
                dataByte|=0x8000; //highest bit is here set to bit 31
            }

            highCnt=0;

            dataByte=dataByte>>1;
            bitCnt++;
            if (bitCnt>=maxBitCnt) {
                // this bit IS STOP BIT... 
                *timeToNextStepIn_ns= -1; //nothing more please
                rxState= RX_WAIT_LOWEDGE;

                /// @todo This is bug if frame format is different (eg 7 or 9 bits)
                unsigned char c=(unsigned char)((dataByte>>(16-maxBitCnt))&0xff); 
                CharReceived(c);
            } else {
                *timeToNextStepIn_ns= (SystemClockOffset)1e9/baudrate/16*(7+7); //read middle of next bit
                rxState=RX_READ_DATABIT_FIRST;
            }

            break;

        default:
            break;
    }

    return 0;
}

void SerialRxBasic::SetBaudRate(SystemClockOffset baud){
    baudrate = baud;
}

void SerialRxBasic::SetHexOutput(bool newValue){
    sendInHex = newValue;
}


// ===========================================================================
// ===========================================================================
// ===========================================================================

void SerialRxBuffered::CharReceived(unsigned char c){
    buffer.push_back(c);
}

unsigned char SerialRxBuffered::Get(){
    unsigned char c = buffer[0];
    buffer.erase(buffer.begin());
    return c;
}

long SerialRxBuffered::Size(){
    return buffer.size();
}


// ===========================================================================
// ===========================================================================
// ===========================================================================


SerialRx::SerialRx(UserInterface *_ui, const char *_name, const char *baseWindow):
ui(_ui), name(_name)  {
    rx.RegisterCallback(this);

    ostringstream os;
    os << "create SerialRx " << name  << " " << baseWindow << endl;
    ui->Write(os.str());
    ui->AddExternalType(name, this);
    Reset();

    /*
    ui->SendUiNewState(name, 't');
    ui->SendUiNewState(name, 'e');
    ui->SendUiNewState(name, 's');
    ui->SendUiNewState(name, 't');
    */
}

void SerialRx::CharReceived(unsigned char c){
    ostringstream os;

    os << "set" << " " << name << " ";
    if (sendInHex) {
        os << hex << "0x" << (unsigned int)c;
    } else if (isprint(c)) {
        if (isspace(c)) {
            os << '_';
        } else {
            os << c;
        }
    } else {
        os << "0x" << (unsigned int)c;
    }

    os << endl;
    ui->Write(os.str());
}


//not used
void SerialRx::SetNewValueFromUi(const string &){
}
