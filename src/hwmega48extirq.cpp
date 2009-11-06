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
#include "avrdevice.h"
#include "hwmega48extirq.h"
#include "irqsystem.h"

HWMega48ExtIrq::HWMega48ExtIrq(AvrDevice *core, HWIrqSystem *i,
        PinAtPort p0, PinAtPort p1,
        unsigned int iv0, unsigned int iv1):
    Hardware(core),
    TraceValueRegister(core, "EXTIRQ"),
    irqSystem(i),
    eicra_reg(this, "EICRA",
              this, &HWMega48ExtIrq::GetEicra, &HWMega48ExtIrq::SetEicra),
    eicrb_reg(this, "EICRB",
              this, &HWMega48ExtIrq::GetEicrb, &HWMega48ExtIrq::SetEicrb),
    eimsk_reg(this, "EIMSK",
              this, &HWMega48ExtIrq::GetEimsk, &HWMega48ExtIrq::SetEimsk),
    eifr_reg(this, "EIFR",
             this, &HWMega48ExtIrq::GetEifr, &HWMega48ExtIrq::SetEifr) {
    pinI.push_back(p0);
    pinI.push_back(p1);

    vectorInt.push_back(iv0);
    vectorInt.push_back(iv1);

    core->AddToCycleList(this);

    Reset();
}

void HWMega48ExtIrq::Reset() {
    eimsk=0;
    eifr=0;
    eicra=0;
    eicrb=0;
    for (int tt=0; tt<2; tt++) { int_old[tt]= pinI[tt]; }
}


unsigned char  HWMega48ExtIrq::GetEimsk(){ return eimsk;}
unsigned char  HWMega48ExtIrq::GetEifr(){return eifr;}
unsigned char HWMega48ExtIrq::GetEicra() { return eicra;}
unsigned char HWMega48ExtIrq::GetEicrb() { return eicrb;}

void  HWMega48ExtIrq::SetEicra(unsigned char val){ eicra=val;}
void  HWMega48ExtIrq::SetEicrb(unsigned char val){ eicrb=val;}

void HWMega48ExtIrq::CheckForNewSetIrq(unsigned char giac) {
    for (int tt =0; tt<2; tt++) {
        if (giac&(1<<tt)) { irqSystem->SetIrqFlag(this, vectorInt[tt]); } 
    }
}


void HWMega48ExtIrq::CheckForNewClearIrq(unsigned char giac) {
    for (int tt =0; tt<2; tt++) {
        if (giac&(1<<tt)) { irqSystem->ClearIrqFlag(vectorInt[tt]); }
    }
}


void  HWMega48ExtIrq::SetEimsk(unsigned char val){
    unsigned char giacOld= eimsk&eifr;
    eimsk=val;
    unsigned char giacNew= eimsk&eifr;

    unsigned char changed=giacNew^giacOld;
    unsigned char setnew= changed&giacNew;
    unsigned char clearnew= changed& (~giacNew);

    CheckForNewSetIrq(setnew);
    CheckForNewClearIrq(clearnew);
}

void HWMega48ExtIrq::SetEifr(unsigned char val){ 
    unsigned char giacOld= eimsk&eifr;
    val^=0xff; //invert the bits, if set 1 clear the flag
    eifr&=val;
    unsigned char giacNew= eimsk&eifr;

    unsigned char changed=giacNew^giacOld;
    unsigned char setnew= changed&giacNew;
    unsigned char clearnew= changed& (~giacNew);

    CheckForNewSetIrq(setnew);
    CheckForNewClearIrq(clearnew);
}

void HWMega48ExtIrq::ClearIrqFlag(unsigned int vector) {
    for (int tt =0; tt<2; tt++) {
        if (vector== vectorInt[tt]) {
            eifr&=0xff-(1<<tt);
            irqSystem->ClearIrqFlag( vectorInt[tt]);
        }
    }
}


unsigned int HWMega48ExtIrq::CpuCycle(){
#ifndef NEW
    PinStateHasChanged(NULL);
#endif
    return 0;
}

void HWMega48ExtIrq::PinStateHasChanged(Pin *p) {
    unsigned char eicr;
	eicr=eicra;

    for (int tt =0; tt<2; tt++) {
        unsigned char actualPin= tt;
        unsigned int actVec=vectorInt[actualPin];
        switch ((eicr>>(tt<<1))&0x03) {
            case 0x00:
                if (pinI[actualPin]==0) {
                    eifr|=(1<<actualPin);
                    if (eimsk& (1<<actualPin)) irqSystem->SetIrqFlag(this, actVec);
                } else {                                //only clear irq flag if not set before!
                    if ((eifr & (1<<actualPin) ) !=0) { //interrupt flag was set
                        eifr=0xff-(1<<actualPin);       //clear flag when pin is 0!
                        irqSystem->ClearIrqFlag(actVec);
                    }
                }
                break;
            case 0x01:
                if (pinI[actualPin]!= int_old[actualPin]) {
                    int_old[actualPin]=pinI[actualPin];
                    eifr|=(1<<actualPin);
                    if (eimsk& (1<<actualPin)) irqSystem->SetIrqFlag(this, actVec);
                }
                break;
            case 0x02:
                if ((pinI[actualPin]==0) && ( int_old[actualPin]==1)) {
                    int_old[actualPin]=0;
                    eifr|=(1<<actualPin);
                    if (eimsk& (1<<actualPin)) irqSystem->SetIrqFlag(this, actVec);
                }
                break;
            case 0x03:
                if ((pinI[actualPin]==1) && ( int_old[actualPin]==0)) {
                    int_old[actualPin]=1;
                    eifr|=(1<<actualPin);
                    if (eimsk& (1<<actualPin)) irqSystem->SetIrqFlag(this, actVec);
                }
                break;
        }
    }

    //return 0;
}		

