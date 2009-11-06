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

#include "avrdevice.h"
#include "hwmegaextirq.h"
#include "irqsystem.h"

//#define NEW


HWMegaExtIrq::HWMegaExtIrq(AvrDevice *core, HWIrqSystem *i,
        PinAtPort p0, PinAtPort p1, PinAtPort p2, PinAtPort p3,
        PinAtPort p4, PinAtPort p5, PinAtPort p6, PinAtPort p7,
        unsigned int iv0, unsigned int iv1, unsigned int iv2, unsigned int iv3,
        unsigned int iv4, unsigned int iv5, unsigned int iv6, unsigned int iv7):
    Hardware(core),
    TraceValueRegister(core, "EXTIRQ"),
    irqSystem(i),
    eimsk_reg(this, "EIMSK",
              this, &HWMegaExtIrq::GetEimsk, &HWMegaExtIrq::SetEimsk),
    eifr_reg(this, "EIFR",
             this, &HWMegaExtIrq::GetEifr, &HWMegaExtIrq::SetEifr),
    eicra_reg(this, "EICRA",
              this, &HWMegaExtIrq::GetEicra, &HWMegaExtIrq::SetEicra),
    eicrb_reg(this, "EICRB",
              this, &HWMegaExtIrq::GetEicrb, &HWMegaExtIrq::SetEicrb) {
    pinI.push_back(p0);
    pinI.push_back(p1);
    pinI.push_back(p2);
    pinI.push_back(p3);
    pinI.push_back(p4);
    pinI.push_back(p5);
    pinI.push_back(p6);
    pinI.push_back(p7);

    vectorInt.push_back(iv0);
    vectorInt.push_back(iv1);
    vectorInt.push_back(iv2);
    vectorInt.push_back(iv3);
    vectorInt.push_back(iv4);
    vectorInt.push_back(iv5);
    vectorInt.push_back(iv6);
    vectorInt.push_back(iv7);

#ifdef NEW
    p0.GetPin().RegisterCallback(this);
    p1.GetPin().RegisterCallback(this);
    p2.GetPin().RegisterCallback(this);
    p3.GetPin().RegisterCallback(this);
    p4.GetPin().RegisterCallback(this);
    p5.GetPin().RegisterCallback(this);
    p6.GetPin().RegisterCallback(this);
    p7.GetPin().RegisterCallback(this);
#else
    core->AddToCycleList(this);
#endif

    Reset();
}

void HWMegaExtIrq::Reset() {
    eimsk=0;
    eifr=0;
    eicra=0;
    eicrb=0;
    for (int tt=0; tt<8; tt++) { int_old[tt]= pinI[tt]; }
}


unsigned char  HWMegaExtIrq::GetEimsk(){ return eimsk;}
unsigned char  HWMegaExtIrq::GetEifr(){return eifr;}
unsigned char HWMegaExtIrq::GetEicra() { return eicra;}
unsigned char HWMegaExtIrq::GetEicrb() { return eicrb;}

void  HWMegaExtIrq::SetEicra(unsigned char val){ eicra=val;}
void  HWMegaExtIrq::SetEicrb(unsigned char val){ eicrb=val;}

void HWMegaExtIrq::CheckForNewSetIrq(unsigned char giac) {
    for (int tt =0; tt<8; tt++) {
        if (giac&(1<<tt)) { irqSystem->SetIrqFlag(this, vectorInt[tt]); } 
    }
}


void HWMegaExtIrq::CheckForNewClearIrq(unsigned char giac) {
    for (int tt =0; tt<8; tt++) {
        if (giac&(1<<tt)) { irqSystem->ClearIrqFlag(vectorInt[tt]); }
    }
}


void  HWMegaExtIrq::SetEimsk(unsigned char val){
    unsigned char giacOld= eimsk&eifr;
    eimsk=val;
    unsigned char giacNew= eimsk&eifr;

    unsigned char changed=giacNew^giacOld;
    unsigned char setnew= changed&giacNew;
    unsigned char clearnew= changed& (~giacNew);

    CheckForNewSetIrq(setnew);
    CheckForNewClearIrq(clearnew);
}

void HWMegaExtIrq::SetEifr(unsigned char val){ 
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

void HWMegaExtIrq::ClearIrqFlag(unsigned int vector) {
    for (int tt =0; tt<8; tt++) {
        if (vector== vectorInt[tt]) {
            eifr&=0xff-(1<<tt);
            irqSystem->ClearIrqFlag( vectorInt[tt]);
        }
    }
}


unsigned int HWMegaExtIrq::CpuCycle(){
#ifndef NEW
    PinStateHasChanged(NULL);
#endif
    return 0;
}

void HWMegaExtIrq::PinStateHasChanged(Pin *p) {
    for (int xx =0; xx<2; xx++) { // we run for eicr(a+b) :-)
        unsigned char eicr;
        unsigned char offset;
        if (xx==0) {
            eicr=eicra;
            offset=0;
        } else {
            eicr= eicrb;
            offset = 4;
        }


        for (int tt =0; tt<4; tt++) {
            unsigned char actualPin= tt+offset;
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
                        eifr|=(1<<actualPin);
                        if (eimsk& (1<<actualPin)) irqSystem->SetIrqFlag(this, actVec);
                    }
                    break;

                case 0x02:
                    if ((pinI[actualPin]==0) && ( int_old[actualPin]==1)) {
                        eifr|=(1<<actualPin);
                        if (eimsk& (1<<actualPin)) irqSystem->SetIrqFlag(this, actVec);
                    }
                    break;

                case 0x03:
                    if ((pinI[actualPin]==1) && ( int_old[actualPin]==0)) {
                        eifr|=(1<<actualPin);
                        if (eimsk& (1<<actualPin)) irqSystem->SetIrqFlag(this, actVec);
                    }
                    break;
            }

            //update the "old state" for next edge detection (patch 5182, thanks to Mike Felser!)
            int_old[actualPin]=pinI[actualPin];

        } //2*4 pins
    } //xx eicra/b

    //return 0;
}		

