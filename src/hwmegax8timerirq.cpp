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
#include <iostream>
using namespace std;
#include "hwmegax8timerirq.h"
#include "irqsystem.h"


/* Timer/Counter Interrupt Mask/Flag register */
#define OCB_BIT  2
#define OCA_BIT  1
#define TOV_BIT   0

HWMegaX8TimerIrq::HWMegaX8TimerIrq(	AvrDevice*		core,
									HWIrqSystem*	is,
									unsigned int	ovfVect,
									unsigned int	compAVect,
									unsigned int	compBVect
									):
	Hardware(core),
	irqSystem(is), 
    tifr(0),
    timsk(0),
    vectorOvf(ovfVect),
    vectorCompA(compAVect),
    vectorCompB(compBVect)
{
}

void HWMegaX8TimerIrq::AddFlagToTifr(unsigned char val){
    tifr|=val;

    switch (val&timsk) 
    {
        case 0: break; //nothing to set, (flag & mask) -> 0
        case (1<<OCB_BIT): { irqSystem->SetIrqFlag(this, vectorCompB); }   break;
        case (1<<OCA_BIT): { irqSystem->SetIrqFlag(this, vectorCompA); }   break;
        case (1<<TOV_BIT): { irqSystem->SetIrqFlag(this, vectorOvf); }    break;
        default: cerr<< "HWMegaX8TimerIrq::AddFlagToTifr" << ":WrongValue for AddFlag 0x"<<hex<<(unsigned int)val<< endl; exit(0);
    }
}

void HWMegaX8TimerIrq::CheckForNewSetIrq(unsigned char tiac) {
    if (tiac&(1<<OCA_BIT)) { irqSystem->SetIrqFlag(this, vectorCompA); }  
    if (tiac&(1<<OCB_BIT)) { irqSystem->SetIrqFlag(this, vectorCompB); }  
    if (tiac&(1<<TOV_BIT)) { irqSystem->SetIrqFlag(this, vectorOvf); } 
}

void HWMegaX8TimerIrq::CheckForNewClearIrq(unsigned char tiac) {
    if (tiac&(1<<OCA_BIT))  { irqSystem->ClearIrqFlag(vectorCompA); }
    if (tiac&(1<<OCB_BIT))  { irqSystem->ClearIrqFlag(vectorCompB); }
    if (tiac&(1<<TOV_BIT))  { irqSystem->ClearIrqFlag(vectorOvf); }
}

void HWMegaX8TimerIrq::ClearIrqFlag(unsigned int vector) {
    if (vector == vectorCompA ) {tifr&=0xff-(1<<OCA_BIT);irqSystem->ClearIrqFlag(vectorCompA);}
    if (vector == vectorCompB ) {tifr&=0xff-(1<<OCB_BIT);irqSystem->ClearIrqFlag(vectorCompB);}
    if (vector == vectorOvf) {tifr&=0xff-(1<<TOV_BIT);irqSystem->ClearIrqFlag(vectorOvf);}
}

unsigned char HWMegaX8TimerIrq::GetTimsk() { return timsk;}
unsigned char HWMegaX8TimerIrq::GetTifr() {return tifr;}

void HWMegaX8TimerIrq::SetTimsk(unsigned char val) {
    unsigned char tiacOld= timsk&tifr;
    timsk=val;
    unsigned char tiacNew= timsk&tifr;

    unsigned char changed=tiacNew^tiacOld;
    unsigned char setnew= changed&tiacNew;
    unsigned char clearnew= changed& (~tiacNew);

    CheckForNewSetIrq(setnew);
    CheckForNewClearIrq(clearnew);
}

void HWMegaX8TimerIrq::SetTifr(unsigned char val) { 
    unsigned char tiacOld= timsk&tifr;
    tifr&=~val;
    unsigned char tiacNew= timsk&tifr;

    unsigned char changed=tiacNew^tiacOld;
    unsigned char clearnew= changed& (~tiacNew);

    CheckForNewClearIrq(clearnew);
}

