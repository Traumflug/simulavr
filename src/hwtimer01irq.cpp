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

#include "hwtimer01irq.h"
#include "irqsystem.h"

#include <iostream>
using namespace std;

HWTimer01Irq::HWTimer01Irq(AvrDevice *core, HWIrqSystem *is, unsigned int v1, unsigned int v2,unsigned int v3,unsigned int v4,unsigned int v5    ):
Hardware(core), irqSystem(is), vectorCapt(v1), vectorCompa(v2), vectorCompb(v3), vectorOvf1(v4), vectorOvf0(v5) {

    TOIE1=1<<7;
    OCIE1A=1<<6;
    OCIE1B=1<<5;
    TICIE1=1<<3;
    TOIE0=1<<1;
    
    TOV1=1<<7;
    OCF1A=1<<6;
    OCF1B=1<<5;
    ICF1=1<<3;
    TOV0=1<<1;
    
    tifr=0;
    timsk=0;
}

void HWTimer01Irq::CheckForNewSetIrq(unsigned char tiac) {
    if (tiac&TICIE1) { irqSystem->SetIrqFlag(this, vectorCapt); } 
    if (tiac&OCIE1A) { irqSystem->SetIrqFlag(this, vectorCompa); }
    if (tiac&OCIE1B) { irqSystem->SetIrqFlag(this, vectorCompb); }
    if (tiac&TOIE1)  { irqSystem->SetIrqFlag(this, vectorOvf1); } 
    if (tiac&TOIE0)  { irqSystem->SetIrqFlag(this, vectorOvf0); } 
}

void HWTimer01Irq::CheckForNewClearIrq(unsigned char tiac) {
    if (tiac&TICIE1) { irqSystem->ClearIrqFlag(vectorCapt); }
    if (tiac&OCIE1A) { irqSystem->ClearIrqFlag(vectorCompa); }
    if (tiac&OCIE1B) { irqSystem->ClearIrqFlag(vectorCompb); }
    if (tiac&TOIE1)  { irqSystem->ClearIrqFlag(vectorOvf1); }
    if (tiac&TOIE0)  { irqSystem->ClearIrqFlag(vectorOvf0); }
}



void HWTimer01Irq::ClearIrqFlag(unsigned int vector) {
    if (vector == vectorCapt)   {tifr&=~TICIE1; irqSystem->ClearIrqFlag(vectorCapt);}
    if (vector == vectorCompa ) {tifr&=~OCIE1A; irqSystem->ClearIrqFlag(vectorCompa);}
    if (vector == vectorCompb ) {tifr&=~OCIE1B; irqSystem->ClearIrqFlag(vectorCompb);}
    if (vector == vectorOvf1 )  {tifr&=~TOIE1;  irqSystem->ClearIrqFlag(vectorOvf1);}
    if (vector == vectorOvf0 )  {tifr&=~TOIE0;  irqSystem->ClearIrqFlag(vectorOvf0);}
}


void HWTimer01Irq::AddFlagToTifr(unsigned char val){
    tifr|=val; 

    unsigned char masked=(val&timsk);
    if (masked==0) {} //nothing to set, (flag & mask) -> 0
    else if (masked==TICIE1) { irqSystem->SetIrqFlag(this, vectorCapt); }
    else if (masked==OCIE1A) { irqSystem->SetIrqFlag(this, vectorCompa);}
    else if (masked==OCIE1B) { irqSystem->SetIrqFlag(this, vectorCompb);}
    else if (masked==TOIE1)  { irqSystem->SetIrqFlag(this, vectorOvf1); }
    else if (masked==TOIE0)  { irqSystem->SetIrqFlag(this, vectorOvf0); }
    else {
	cerr << "HWTimer01Irq::AddFlagToTifr: Wrong Value For AddFlag 0x"<<hex << (unsigned int) val << endl;
    }
}


unsigned char HWTimer01Irq::GetTimsk() { return timsk;}
unsigned char HWTimer01Irq::GetTifr() {return tifr;}

void HWTimer01Irq::SetTimsk(unsigned char val) { 
    unsigned char tiacOld= timsk&tifr;
    timsk=val;
    unsigned char tiacNew= timsk&tifr;

    unsigned char changed=tiacNew^tiacOld;
    unsigned char setnew= changed&tiacNew;
    unsigned char clearnew= changed& (~tiacNew);

    CheckForNewSetIrq(setnew);
    CheckForNewClearIrq(clearnew);
}

void HWTimer01Irq::SetTifr(unsigned char val) { 
    unsigned char tiacOld= timsk&tifr;
    tifr&=~val;
    unsigned char tiacNew= timsk&tifr;

    unsigned char changed=tiacNew^tiacOld;
    unsigned char clearnew= changed& (~tiacNew);

    CheckForNewClearIrq(clearnew);
}

