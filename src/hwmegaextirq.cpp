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
#include "hwmegaextirq.h"
#include "irqsystem.h"
#include "trace.h"

HWMegaExtIrq::HWMegaExtIrq(AvrDevice *core, HWIrqSystem *i,
        PinAtPort p0, PinAtPort p1, PinAtPort p2, PinAtPort p3,
        PinAtPort p4, PinAtPort p5, PinAtPort p6, PinAtPort p7,
        unsigned int iv0, unsigned int iv1, unsigned int iv2, unsigned int iv3,
        unsigned int iv4, unsigned int iv5, unsigned int iv6, unsigned int iv7):
Hardware(core), irqSystem(i)
{
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



    core->AddToCycleList(this);
    /*
    irqSystem->RegisterIrqPartner(this, iv0);
    irqSystem->RegisterIrqPartner(this, iv1);
    irqSystem->RegisterIrqPartner(this, iv2);
    irqSystem->RegisterIrqPartner(this, iv3);
    irqSystem->RegisterIrqPartner(this, iv4);
    irqSystem->RegisterIrqPartner(this, iv5);
    irqSystem->RegisterIrqPartner(this, iv6);
    irqSystem->RegisterIrqPartner(this, iv7);
    */
    eimsk=0;
    eifr=0;
    eicra=0;
    eicrb=0;
}

unsigned char  HWMegaExtIrq::GetEimsk(){ return eimsk;}
unsigned char  HWMegaExtIrq::GetEifr(){return eifr;}
unsigned char HWMegaExtIrq::GetEicra() { return eicra;}
unsigned char HWMegaExtIrq::GetEicrb() { return eicrb;}

void  HWMegaExtIrq::SetEicra(unsigned char val){ eicra=val;}
void  HWMegaExtIrq::SetEicrb(unsigned char val){ eicrb=val;}

void  HWMegaExtIrq::SetEimsk(unsigned char val){
    eimsk=val;
    CheckForIrq();
}

void HWMegaExtIrq::SetEifr(unsigned char val){ 
    val^=0xff; //invert the bits, if set 1 clear the flag
    eifr&=val;
    CheckForIrq();
}

void HWMegaExtIrq::CheckForIrq() {
    unsigned char giac= eimsk&eifr;

    for (int tt =0; tt<8; tt++) {
        if (giac&(1<<tt)) { 
            irqSystem->SetIrqFlag(this, vectorInt[tt]);
        } else {
            irqSystem->ClearIrqFlag(vectorInt[tt]);
        }
    }
}

#if 0
bool HWMegaExtIrq::IsIrqFlagSet(unsigned int vector) {
    cout << "MEGAEXTIRQ IsIrqFlag was called" << endl;
    return 1;

    /* function should be removed XXX !!!! 

    unsigned char giac= eimsk&eifr;
    if (giac!=0) { //that should make the compare a bit faster :-), while a setted
        //flag should be seldom

        for (int tt =0; tt<8; tt++) {
            if (vector== vectorInt[tt]) {
                if (giac&(1<<tt)) { return 1; }
            }
        }
    }

    return 0; 
    */
}
#endif


void HWMegaExtIrq::ClearIrqFlag(unsigned int vector) {
    for (int tt =0; tt<8; tt++) {
        if (vector== vectorInt[tt]) {
            eifr&=0xff-(1<<tt);
            irqSystem->ClearIrqFlag( vectorInt[tt]);
        }
    }
}


unsigned int HWMegaExtIrq::CpuCycle(){
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
                    } else {
                        eifr=0xff-(1<<actualPin); //clear flag when pin is 0!
                        irqSystem->ClearIrqFlag(actVec);
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
        } //2*4 pins
    } //xx eicra/b

    return 0;
}		


unsigned char RWEicra::operator=(unsigned char val) { trioaccess("Eicra",val); megaextirq->SetEicra(val); return val; }
unsigned char RWEicrb::operator=(unsigned char val) { trioaccess("Eicrb",val); megaextirq->SetEicrb(val); return val; }
unsigned char RWEimsk::operator=(unsigned char val) { trioaccess("Eimsk",val); megaextirq->SetEimsk(val); return val; }
unsigned char RWEifr::operator=(unsigned char val) { trioaccess("Eifr",val); megaextirq->SetEifr(val); return val; }
RWEicra::operator unsigned char() const { return megaextirq->GetEicra(); }
RWEicrb::operator unsigned char() const { return megaextirq->GetEicrb(); }
RWEimsk::operator unsigned char() const { return megaextirq->GetEimsk(); }
RWEifr::operator unsigned char() const { return megaextirq->GetEifr(); }
