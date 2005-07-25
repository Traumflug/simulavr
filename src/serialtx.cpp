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


SerialTxBuffered::SerialTxBuffered(){
	allPins["tx"] = &tx;
	Reset();
};

void SerialTxBuffered::Reset(){
	txState=TX_DISABLED;
    baudrate=115200;
    maxBitCnt=8;
    tx='H'; 
};

Pin* SerialTxBuffered::GetPin(const char* name){
	return allPins[name];
}

int SerialTxBuffered::Step(bool &trueHwStep, SystemClockOffset *timeToNextStepIn_ns){
    switch (txState) {
        case TX_SEND_STARTBIT:
            data=*(inputBuffer.begin());
            inputBuffer.erase(inputBuffer.begin());
            tx='L';
            bitCnt=0;
            *timeToNextStepIn_ns=(SystemClockOffset)1e9/baudrate; //all we measures are in ns !;
            txState=TX_SEND_DATABIT;
            break;

        case TX_SEND_DATABIT:
            if (( data >> bitCnt ) & 0x01)  {
                tx='H';
            } else {
                tx='L';
            }
            *timeToNextStepIn_ns=(SystemClockOffset)1e9/baudrate; //all we measures are in ns !;
            bitCnt++;
            if(bitCnt>=maxBitCnt) txState=TX_SEND_STOPBIT;
            break;

        case TX_SEND_STOPBIT:
            tx='H';
            txState=TX_STOPPING;
            *timeToNextStepIn_ns=(SystemClockOffset)1e9/baudrate; //last step, do not continue
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
            cerr << "Illegal state in SerialTx" << endl;
            exit(0);
    }


    return 0;

}

void SerialTxBuffered::Send(unsigned char data){
	inputBuffer.push_back(data); //write new char to input buffer

    //if we not active, activate tx machine now
    if (txState==TX_DISABLED) {
        txState=TX_SEND_STARTBIT;
        SystemClock::Instance().Add(this);
    }
};

void SerialTxBuffered::SetBaudRate(SystemClockOffset baud){
	baudrate = baud;
};






// ===========================================================================
// ===========================================================================
// ===========================================================================

// don't know why is this needed, but don't want to change it.. just in case..
unsigned int SerialTx::CpuCycle() {
    CpuCycleTx();

    return 0;
}

unsigned int SerialTx::CpuCycleTx() {
    return 0;
}

SerialTx::SerialTx(UserInterface *_ui, const char *_name, const char *baseWindow):
ui(_ui), name(_name)  {
    ostringstream os;
    os << "create SerialTx " << name  << " " << baseWindow << endl;
    ui->Write(os.str());
    ui->AddExternalType(name, this);
    Reset();
}

void SerialTx::SetNewValueFromUi(const string &s) {
    Send(s[0]);
}
