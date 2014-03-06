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

#include "irqsystem.h"
#include "avrdevice.h"
#include "funktor.h"
#include "systemclock.h"
#include "helper.h"
#include "avrerror.h"

#include "application.h"

#include <iostream>
#include <assert.h>
#include <typeinfo>

using namespace std;

// global switch to enable irq statistic (default is disabled)
bool enableIRQStatistic = false;

// if HEXOUT is set the statistic output will be written in hex
// if not defined the output is decimal seperated with tabs for reading it with gnumeric
//#define HEXOUT

void IrqStatisticEntry::CalcDiffs() {
    setClear          =flagCleared-flagSet;
    setStarted        =handlerStarted-flagSet;
    setFinished       =handlerFinished-flagSet;
    startedFinished   =handlerFinished-handlerStarted;
}

IrqStatisticPerVector::IrqStatisticPerVector() {
    // set the "short" params to the max values so that the first "not dummy" is smaller 
    //  and set to the real statistic
    IrqStatisticEntry longDummy;
    IrqStatisticEntry shortDummy;

    longDummy.setClear=0xffffffffffffll;
    longDummy.setStarted=0xffffffffffffll;
    longDummy.setFinished=0xffffffffffffll;
    longDummy.startedFinished=0xffffffffffffll;

    shortDummy.setClear=0;
    shortDummy.setStarted=0;
    shortDummy.setFinished=0;
    shortDummy.startedFinished=0;

    long_SetClear=shortDummy;
    long_SetStarted=shortDummy;
    long_SetFinished=shortDummy;
    long_StartedFinished=shortDummy;

    short_SetClear=longDummy;
    short_SetStarted=longDummy;
    short_SetFinished=longDummy;
    short_StartedFinished=longDummy;
}

void IrqStatisticPerVector::CalculateStatistic() {
    actual.CalcDiffs();

    if (actual.setClear< short_SetClear.setClear) {
        short_SetClear=actual;
    }

    if (actual.setClear> long_SetClear.setClear) {
        long_SetClear=actual;
    }

    if (actual.setStarted< short_SetStarted.setStarted) {
        short_SetStarted=actual;
    }

    if( actual.setStarted> long_SetStarted.setStarted) {
        long_SetStarted=actual;
    }

    if ( actual.setFinished< short_SetFinished.setFinished) {
        short_SetFinished= actual;
    }

    if ( actual.setFinished > long_SetFinished.setFinished) {
        long_SetFinished= actual;
    }

    if (actual.startedFinished < short_StartedFinished.startedFinished) {
        short_StartedFinished= actual;
    }

    if (actual.startedFinished > long_StartedFinished.startedFinished) {
        long_StartedFinished= actual;
    }
}

void IrqStatisticPerVector::CheckComplete() {
    if ((actual.flagSet!=0) &&
            (actual.flagCleared!=0) &&
            (actual.handlerStarted!=0) &&
            (actual.handlerFinished!=0)) 
    {
        CalculateStatistic();
        IrqStatisticEntry emptyDummy; 
        actual=emptyDummy;

    }
}

ostream &helpHexOut(ostream &os, unsigned long long x) {
#ifdef HEXOUT
    os << "0x";
    os.fill('0');
    os.width(16);
    os << hex << x << ":" ;
#else
    os<< x << "\t" ;
#endif
    return os;
}

ostream& operator<<(ostream &os, const IrqStatisticEntry& ise) {
    os << dec<<"\t";
    helpHexOut(os, ise.flagSet) ;
    helpHexOut(os, ise.flagCleared ); 
    helpHexOut(os, ise.handlerStarted ); 
    helpHexOut(os, ise.handlerFinished ); 
    helpHexOut(os, ise.setClear ); 
    helpHexOut(os, ise.setStarted ); 
    helpHexOut(os, ise.setFinished ); 
    helpHexOut(os, ise.startedFinished ); 

    return os;
}

ostream& operator<<(ostream &os, const IrqStatisticPerVector &ispv) {
    os << "Set->Clear >" << ispv.long_SetClear << endl; 
    os << "Set->Clear <" << ispv.short_SetClear << endl;
    os << "Set->HandlerStarted >" << ispv.long_SetStarted << endl; 
    os << "Set->HandlerStarted <" << ispv.short_SetStarted << endl;

    os << "Set->HandlerFinished >" << ispv.long_SetFinished << endl; 
    os << "Set->HandlerFinished <" << ispv.short_SetFinished << endl;
    os << "Handler Start->Finished >" << ispv.long_StartedFinished << endl; 
    os << "Handler Start->Finished <" << ispv.short_StartedFinished << endl;

    return os;
}


ostream& operator<<(ostream &os, const IrqStatistic& is) {
    map<unsigned int, IrqStatisticPerVector>::const_iterator ii;

    os << "IRQ STATISTIC" << endl;
    os << "\tFlagSet\tflagCleared\tHandlerStarted\tHandlerFinished\tSet->Clear\tSet->Started\tSet->Finished\tStarted->Finished"<<endl;
    for (ii=is.entries.begin(); ii!= is.entries.end(); ii++) {
        os << "Core: "  << is.core->GetFname() <<endl;
        os << "Statistic for vector: 0x" << hex << ii->first << endl;
        os << (ii->second);
    }

    return os;
}

HWIrqSystem::HWIrqSystem(AvrDevice* _core, int bytes, int tblsize):
    TraceValueRegister(_core, "IRQ"),
    bytesPerVector(bytes),
    vectorTableSize(tblsize),
    irqTrace(tblsize),
    core(_core),
    irqStatistic(_core),
    debugInterruptTable(tblsize, (Hardware*)NULL)
{
    for(unsigned int i = 0; i < vectorTableSize; i++) {
        TraceValue* tv = new TraceValue(1, GetTraceValuePrefix() + "VECTOR" + int2str(i));
        tv->set_written(0);
        RegisterTraceValue(tv);
        irqTrace[i] = tv;
    }
}

unsigned int HWIrqSystem::GetNewPc(unsigned int &actualVector) {
    unsigned int newPC = 0xffffffff;

    static map<unsigned int, Hardware *>::iterator ii;
    static map<unsigned int, Hardware *>::iterator end;
    end = irqPartnerList.end();

    //a map is always sorted, so the priority of the irq vector is known and handled correctly
    for(ii = irqPartnerList.begin(); ii != end; ii++) {
        Hardware* second = ii->second;
        unsigned int index = ii->first;
        assert(index < vectorTableSize);

        if(second->IsLevelInterrupt(index)) {
            second->ClearIrqFlag(index);
            if(second->LevelInterruptPending(index)) {
                actualVector = index;
                newPC = index * (bytesPerVector / 2);
                break;
            }
        } else {
            second->ClearIrqFlag(index);
            actualVector = index;
            newPC = index * (bytesPerVector / 2);
            break;
        }
    }

    return newPC;
}

void HWIrqSystem::SetIrqFlag(Hardware *hwp, unsigned int vector) {
    assert(vector < vectorTableSize);
    irqPartnerList[vector]=hwp;
    if (core->trace_on) {
        traceOut << core->GetFname() << " interrupt on index " << vector << " is pending" << endl;
    }

    if ( irqStatistic.entries[vector].actual.flagSet==0) { //the actual entry was not used before... fine!
        irqStatistic.entries[vector].actual.flagSet=SystemClock::Instance().GetCurrentTime();
    } 
}

void HWIrqSystem::ClearIrqFlag(unsigned int vector) {
    irqPartnerList.erase(vector);
    if (core->trace_on) {
        traceOut << core->GetFname() << " interrupt on index " << vector << "cleared" << endl;
    }

    if (irqStatistic.entries[vector].actual.flagCleared==0) {
        irqStatistic.entries[vector].actual.flagCleared=SystemClock::Instance().GetCurrentTime();
    }

    irqStatistic.entries[vector].CheckComplete();
} 

void HWIrqSystem::IrqHandlerStarted(unsigned int vector) {
    irqTrace[vector]->change(1);
    if (core->trace_on) {
        traceOut << core->GetFname() << " IrqSystem: IrqHandlerStarted Vec: " << vector << endl;
    }

    if (irqStatistic.entries[vector].actual.handlerStarted==0) {
        irqStatistic.entries[vector].actual.handlerStarted=SystemClock::Instance().GetCurrentTime();
    }
    irqStatistic.entries[vector].CheckComplete();
}

void HWIrqSystem::IrqHandlerFinished(unsigned int vector) {
    irqTrace[vector]->change(0);
    if (core->trace_on) {
        traceOut << core->GetFname() << " IrqSystem: IrqHandler Finished Vec: " << vector << endl;
    }

    if (irqStatistic.entries[vector].actual.handlerFinished==0) {
        irqStatistic.entries[vector].actual.handlerFinished=SystemClock::Instance().GetCurrentTime();
    }
    irqStatistic.entries[vector].CheckComplete();
}

void HWIrqSystem::DebugVerifyInterruptVector(unsigned int vector, const Hardware* source) {
    assert(vector < vectorTableSize);
    const Hardware* existing = debugInterruptTable[vector];
    if(existing == NULL)
        debugInterruptTable[vector] = source;
    else
        assert(existing == source);
    // The same `source' for multiple `vector' values is OK. It would be pain
    // for ExternalIRQSingle class to inherit from Hardware just for this test.
}
void HWIrqSystem::DebugDumpTable()
{
    avr_message("Interrupt vector table (for comparison against a datasheet)\n");
    avr_message("Vector | Address/2 | Source Peripheral (class)\n");
    for(unsigned i = 0; i < debugInterruptTable.size(); i++)
    {
        const Hardware* source = debugInterruptTable[i];
        const char * handler = (i==0) ? "funct AvrDevice::Reset()"
            : source ? typeid(*source).name() : "(unsupported or not registered)";
        avr_message("  %3d  |   $%04x   | %s\n", i+1, i*bytesPerVector/2, handler);
    }
}

IrqStatistic::IrqStatistic (AvrDevice *c):Printable(cout), core(c) {
    Application::GetInstance()->RegisterPrintable(this);
}

//the standard function object for a printable is printing to "out", so we do this here 
void IrqStatistic::operator()() {
    if(enableIRQStatistic)
        out << *this;
}

