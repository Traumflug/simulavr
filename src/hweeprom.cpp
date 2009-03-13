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

#include "hweeprom.h"
#include "global.h"
#include "avrdevice.h"
#include "systemclock.h"
#include "trace.h"

#include "irqsystem.h"

HWEeprom::HWEeprom(AvrDevice *_core, unsigned int size): Hardware(_core),Memory(size),core(_core) {
    //core->AddToCycleList(this);
    Reset();
}

void HWEeprom::Reset() {
    eecr=0;
    state=READY;
    cpuHoldCycles=0; 

}


HWEeprom::~HWEeprom() {
    free(myMemory);
}

unsigned char HWEeprom::GetEearl() {return eear&0xff; }
unsigned char HWEeprom::GetEearh() {return (eear&0xff00)>>8; }
unsigned char HWEeprom::GetEedr() { return eedr; }
unsigned char HWEeprom::GetEecr() { return eecr; }

void HWEeprom::SetEearl(unsigned char val) {
    eear=eear&0xff00;
    eear+=val;
    if (core->trace_on==1) traceOut << "EEAR=0x"<<hex << eear<<dec;
}

void HWEeprom::SetEearh(unsigned char val) {
    eear=eear&0x00ff;
    eear+=(val<<8);
    if (core->trace_on==1) traceOut << "EEAR=0x"<<hex << eear<<dec;
}

void HWEeprom::SetEedr(unsigned char val) {
    eedr=val;
    if (core->trace_on==1) traceOut << "EEDR=0x"<<hex << (unsigned int) eedr<<dec;
}

void HWEeprom::SetEecr(unsigned char newval) {
    unsigned char localIrqFlag=newval&0x08;
    if (core->trace_on==1) traceOut << "EECR=0x"<<hex << (unsigned int) newval<<dec;


    if (((newval&0x07)==0x04) && (state==READY)) {
        state=WRITE_ENABLED;
        core->AddToCycleList(this);
        writeEnableCycles=4;
        eecr=0x04 | localIrqFlag;
        if (core->trace_on==1) traceOut << " EEPROM: Master Write Enabled ";
        return ;
    }

    if (((newval & 0x03) == 0x02) && (state == WRITE_ENABLED)) {
        state=WRITE;
        cpuHoldCycles=2;
        eecr=0x02 | localIrqFlag;
        writeStartTime=SystemClock::Instance().GetCurrentTime();
        if (core->trace_on==1) traceOut << " EEPROM: Write started ";
        return;
    }

    if ((newval & 0x01) == 0x01) { //EERE EEprom read enable
        if (state==WRITE) {
            state=READY;
            core->RemoveFromCycleList(this);
            eecr=localIrqFlag;
            if (core->trace_on==1) {
                traceOut << " EEPROM Write fails while try to read!" << endl;
            } else {
                cerr << " EEPROM Write fails while try to read!" << endl;
            }
        } else { //read enable
            cpuHoldCycles=4;
            if (state != READ) {
                state=READ;
                core->AddToCycleList(this);
            }

            eecr=0x01 | localIrqFlag;
            if (core->trace_on==1) traceOut << " EEPROM: Read ";

        }
        return;
    }

    if ((newval & 0x07) != 0x00) {
        state=READY;
        core->RemoveFromCycleList(this);
        if( core->trace_on==1) {
            traceOut << "Illegal EEPROM access!" << endl;
        } else {
            cerr<< "Illegal EEPROM access!" << endl;
        }
    }

    state= READY;
    eecr=localIrqFlag;
}

unsigned int HWEeprom::CpuCycle() {
    switch (state) {
        case WRITE_ENABLED:
            writeEnableCycles--;
            if (writeEnableCycles == 0) {
                eecr=0;
                state= READY;
                if (core->trace_on==1) traceOut << " EEPROM: WriteEnable cleared :-( ";
            }
            break;

        case WRITE:
            if (cpuHoldCycles>0) cpuHoldCycles--;
            if (SystemClock::Instance().GetCurrentTime()-writeStartTime > 2500000 ) { //write is ready
                state=READY;
                eecr=0;
                myMemory[eear]=eedr;
                if (core->trace_on==1) traceOut << " EEPROM: Write finished ";
            }
            break;

        case READ:
            cpuHoldCycles--;
            if (cpuHoldCycles==0) {
                eedr=myMemory[eear];
                state=READY;
                eecr=0;
                if (core->trace_on==1) traceOut << "EEPROM["<< hex << eear << "," << GetSymbolAtAddress(eear) << "]->EEDR=0x"<< hex << (unsigned int)myMemory[eear] << " : finished  |" << dec;
            }
            break;

        case READY:
            core->RemoveFromCycleList(this);
            break;

    } //end of switch

    if (cpuHoldCycles>0) return 1;	//let the cpu sleep a cycle
    return 0;						//let the cpu continue


}

void HWEeprom::WriteAtAddress(unsigned int addr, unsigned char val) {
    myMemory[addr]=val;
}

unsigned char HWEeprom::ReadFromAddress( unsigned int addr) {
    return myMemory[addr];
}

void HWEeprom::WriteMem( unsigned char *src, unsigned int offset, unsigned int secSize) {
    for (unsigned int tt=0; tt<secSize; tt++) { 
        if (tt+offset<size) {
            *(myMemory+tt+offset)=src[tt];
        }
    }
}

HWMegaEeprom::HWMegaEeprom(AvrDevice *_core, HWIrqSystem *_irqSystem, unsigned int size, unsigned int irqVec):
HWEeprom( _core, size), irqSystem(_irqSystem), irqVectorNo(irqVec), irqFlag(false) {
    //irqSystem->RegisterIrqPartner(this, irqVectorNo);
}

#if 0
bool HWMegaEeprom::IsIrqFlagSet(unsigned int vector) {
    /* XXX remove this funtion later
    if ((irqFlag==true )&& (vector== irqVectorNo) ) {
        return 1;
    } 
    return 0;
    */
    return 1;
}
#endif

void HWMegaEeprom::ClearIrqFlag(unsigned int vector) {
    if (vector== irqVectorNo) {
        irqFlag=false;
        irqSystem->ClearIrqFlag( irqVectorNo );
    }
}

//TODO set the flags to corrosponding NAMED register bits and 
// correct the irq implementation!!!!
// the irq is only valid if writeEnabled is low end EERIE is one.
// this is currently not implemented. It is possible to remove the "irqFlag" here,
// because this information is dedundant


unsigned int HWMegaEeprom::CpuCycle() {
    switch (state) {
        case WRITE_ENABLED:
            writeEnableCycles--;
            if (writeEnableCycles == 0) {
                eecr&=0xf8;
                state= READY;
                if (core->trace_on==1) traceOut << " EEPROM: WriteEnable cleared :-( ";
            }
            break;

        case WRITE:
            if (cpuHoldCycles>0) cpuHoldCycles--;
            if (SystemClock::Instance().GetCurrentTime()-writeStartTime > 2500000 ) { //write is ready
                state=READY;
                eecr&=0xf8;
                myMemory[eear]=eedr;
                if (core->trace_on==1) traceOut << " EEPROM: Write finished ";
                //now raise irq if enabled
                if ((eecr&0x08)!=0x00) {
                    irqFlag=true;
                    irqSystem->SetIrqFlag(this, irqVectorNo);
                }
            }
            break;

        case READ:
            cpuHoldCycles--;
            if (cpuHoldCycles==0) {
                eedr=myMemory[eear];
                state=READY;
                eecr&=0xf8;
                if (core->trace_on==1) traceOut << "EEPROM["<< hex << eear << "," << GetSymbolAtAddress(eear) << "]->EEDR=0x"<< hex << (unsigned int)myMemory[eear] << " : finished  |" << dec;
            }
            break;

        case READY:
            core->RemoveFromCycleList(this);
            break;


    } //end of switch
    //    traceOut<< endl << "eecr=0x" << hex << (unsigned int) eecr << endl;

    if (cpuHoldCycles>0) return 1;	//let the cpu sleep a cycle
    return 0;						//let the cpu continue


}


unsigned char RWEearl::operator=(unsigned char val) { ee->SetEearl(val); return val;} 
unsigned char RWEearh::operator=(unsigned char val) { ee->SetEearh(val); return val;} 
unsigned char RWEedr::operator=(unsigned char val) { ee->SetEedr(val); return val;} 
unsigned char RWEecr::operator=(unsigned char val) { ee->SetEecr(val); return val;} 
RWEearl::operator unsigned char() const { return ee->GetEearl();  } 
RWEearh::operator unsigned char() const { return ee->GetEearh();  } 
RWEedr::operator unsigned char() const { return ee->GetEedr();  } 
RWEecr::operator unsigned char() const { return ee->GetEecr();  } 
