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
#include "serialtx.h"
#include "systemclock.h"


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

unsigned int SerialTx::CpuCycle() {
    CpuCycleTx();

    return 0;
}

unsigned int SerialTx::CpuCycleTx() {
    return 0;
}

SerialTx::SerialTx(UserInterface *_ui, const char *_name, const char *baseWindow):
ui(_ui), name(_name)  {
    allPins["tx"]= &tx;

    ostringstream os;
    os << "create SerialTx " << name  << " " << baseWindow << endl;
    ui->Write(os.str());
    ui->AddExternalType(name, this);
    Reset();
}

void SerialTx::Reset() {
    txState=TX_DISABLED;
    baudrate=115200;
    maxBitCnt=8;
    tx='H'; 
}

Pin* SerialTx::GetPin(const char *name) {
    return allPins[name];
}

int SerialTx::Step(bool &trueHwStep, SystemClockOffset *timeToNextStepIn_ns){
    switch (txState) {
        case TX_SEND_STARTBIT:
            data=*(inputBuffer.begin());
            inputBuffer.erase(inputBuffer.begin());
            tx='L';
            bitCnt=0;
            *timeToNextStepIn_ns=(SystemClockOffset)1e9/baudrate; //all we measures are in us!;
            txState=TX_SEND_DATABIT;
            break;

        case TX_SEND_DATABIT:
            if (( data >> bitCnt ) & 0x01)  {
                tx='H';
            } else {
                tx='L';
            }
            *timeToNextStepIn_ns=(SystemClockOffset)1e9/baudrate; //all we measures are in us!;
            bitCnt++;
            if(bitCnt>=maxBitCnt) txState=TX_SEND_STOPBIT;
            break;

        case TX_SEND_STOPBIT:
            tx='H';
            if (inputBuffer.size()>0) { //another char to send!
                txState=TX_SEND_STARTBIT;
                *timeToNextStepIn_ns=(SystemClockOffset)1e9/baudrate;
            } else {
                txState=TX_DISABLED;
                *timeToNextStepIn_ns=-1; //last step, do not continue
            }

            break;

        default:
            cerr << "Illegal state in SerialTx" << endl;
            exit(0);
    }


    return 0;

}

void SerialTx::SetNewValueFromUi(const string &s) {
    inputBuffer.push_back(s[0]); //write new char to input buffer

    //if we not active, activate tx machine now
    if (txState==TX_DISABLED) {
        txState=TX_SEND_STARTBIT;
        SystemClock::Instance().Add(this);
    }

}
