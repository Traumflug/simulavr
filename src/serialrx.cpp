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
 */
#include "serialrx.h"
#include "systemclock.h"
#include "systemclocktypes.h"


#define UDRE 0x20
#define TXEN 0x08
#define RXEN 0x10
#define RXB8 0x02
#define FE 0x10
#define CHR9 0x04
#define RXC 0x80
#define TXC 0x40
#define TXB8 0x01

#define RXCIE 0x80
#define TXCIE 0x40
#define UDRIE 0x20

unsigned int SerialRx::CpuCycle() {
    CpuCycleRx();

    return 0;
}
unsigned int SerialRx::CpuCycleRx() {
    return 0;
}

void SerialRx::PinStateHasChanged(Pin *p){
    if (0==(bool)(*p)) { //Low
        if (rxState== RX_WAIT_LOWEDGE) {
            rxState=RX_READ_STARTBIT;
            SystemClock::Instance().Add(this); //as next Step() is called
        }
    }
}


SerialRx::SerialRx(UserInterface *_ui, const char *_name, const char *baseWindow):
ui(_ui), name(_name)  {
    rx.RegisterCallback(this);
    allPins["rx"]= &rx;

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

void SerialRx::Reset() {
    baudrate=115200;
    maxBitCnt=10; //Start+8Data+Stop
    rxState=RX_WAIT_LOWEDGE;
}

Pin* SerialRx::GetPin(const char *name) {
    return allPins[name];
}

int SerialRx::Step(bool &trueHwStep, SystemClockOffset *timeToNextStepIn_ns){
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
            if ((bool)(rx)) {
                highCnt++;
            }
            break;

        case RX_READ_DATABIT_SECOND: //(1/8)
            *timeToNextStepIn_ns= (SystemClockOffset)1e9/baudrate/16;
            rxState= RX_READ_DATABIT_THIRD;
            if ((bool)(rx)) {
                highCnt++;
            }

            break;

        case RX_READ_DATABIT_THIRD: //(1/9)
            rxState= RX_READ_DATABIT_FIRST;
            if ((bool)(rx)) {
                highCnt++;
            }

            if (highCnt>1) {
                dataByte|=0x8000; //highest bit is here set to bit 31
            }

            highCnt=0;

            dataByte=dataByte>>1;
            bitCnt++;
            if (bitCnt>=maxBitCnt) {
                *timeToNextStepIn_ns= (SystemClockOffset)1e9/baudrate/16*7; //only to the end of THIS bit
                rxState=RX_READ_STOPBIT;
            } else {
                *timeToNextStepIn_ns= (SystemClockOffset)1e9/baudrate/16*(7+7); //read first edge of next bit
                rxState=RX_READ_DATABIT_FIRST;
            }


            break;

        case RX_READ_STOPBIT:
            {
                *timeToNextStepIn_ns= -1; //nothing more please
                rxState= RX_WAIT_LOWEDGE;
                ostringstream os;
                unsigned char c=(unsigned char)((dataByte>>6)&0xff);
                if (isprint(c)) {
                    if (isspace(c)) {
                        os << "set" << " " << name << " " << '_' << endl; 
                    } else {
                        os << "set" << " " << name << " " << c << endl; 
                    }
                } else {
                    os << "set" << " " << name << " " << "0x" << hex << (unsigned int)c << endl;
                }

                ui->Write(os.str());
            }
            break;

        default:
            break;
    }

    return 0;

}

//not used
void SerialRx::SetNewValueFromUi(const string &){
}
