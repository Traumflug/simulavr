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

#include "hwmegatimer0123irq.h"
#include "irqsystem.h"


/* Timer/Counter Interrupt MaSK register */
#define    OCF2         7
#define    TOV2         6
#define    ICF1         5
#define    OCF1A        4
#define    OCF1B        3
#define    TOV1         2
#define    OCF0         1
#define    TOV0         0

/* Timer/Counter Interrupt Flag register */
#define    ICF3         5
#define    OCF3A        4
#define    OCF3B        3
#define    TOV3         2
#define    OCF3C        1
#define    OCF1C        0

HWMegaTimer0123Irq::HWMegaTimer0123Irq(AvrDevice *core, HWIrqSystem *is,
        unsigned int t0Comp, 
        unsigned int t0Ovf,
        unsigned int t1compa,
        unsigned int t1compb,
        unsigned int t1compc,
        unsigned int t1capt, 
        unsigned int t1ovf,
        unsigned int t2comp, 
        unsigned int t2ovf,
        unsigned int t3compa,
        unsigned int t3compb,
        unsigned int t3compc,
        unsigned int t3capt, 
        unsigned int t3ovf): 
Hardware(core), irqSystem(is), 
    vectorTimer0Comp( t0Ovf),
    vectorTimer0Ovf( t0Ovf),
    vectorTimer1CompA( t1compa),
    vectorTimer1CompB( t1compb),
    vectorTimer1CompC( t1compc),
    vectorTimer1Capt( t1capt),
    vectorTimer1Ovf( t1ovf),
    vectorTimer2Comp( t2comp),
    vectorTimer2Ovf( t2ovf),
    vectorTimer3CompA(t3compa),
    vectorTimer3CompB(t3compb),
    vectorTimer3CompC(t3compc),
    vectorTimer3Capt(t3capt),
vectorTimer3Ovf(t3ovf)


{	
    /* 
    irqSystem->RegisterIrqPartner(this, vectorTimer0Comp );
    irqSystem->RegisterIrqPartner(this, vectorTimer0Ovf );
    irqSystem->RegisterIrqPartner(this, vectorTimer1CompA );
    irqSystem->RegisterIrqPartner(this, vectorTimer1CompB );
    irqSystem->RegisterIrqPartner(this, vectorTimer1CompC );
    irqSystem->RegisterIrqPartner(this, vectorTimer1Capt );
    irqSystem->RegisterIrqPartner(this, vectorTimer1Ovf );
    irqSystem->RegisterIrqPartner(this, vectorTimer2Comp );
    irqSystem->RegisterIrqPartner(this, vectorTimer2Ovf );
    irqSystem->RegisterIrqPartner(this, vectorTimer3CompA );
    irqSystem->RegisterIrqPartner(this, vectorTimer3CompB );
    irqSystem->RegisterIrqPartner(this, vectorTimer3CompC );
    irqSystem->RegisterIrqPartner(this, vectorTimer3Capt );
    irqSystem->RegisterIrqPartner(this, vectorTimer3Ovf );
    */

    tifr=0;
    timsk=0;
    etimsk=0;
    etifr=0;

}


void HWMegaTimer0123Irq::AddFlagToTifr(unsigned char val){
    tifr|=val;
    CheckForIrq();
}

void HWMegaTimer0123Irq::AddFlagToEtifr(unsigned char val){
    etifr|=val; 
    CheckForIrq();
}

void HWMegaTimer0123Irq::CheckForIrq() {
    unsigned char tiac= timsk&tifr;
    unsigned char etiac= etimsk&etifr;

    if (tiac&(1<<OCF0)) { irqSystem->SetIrqFlag(this, vectorTimer0Comp); }  else { irqSystem->ClearIrqFlag(vectorTimer0Comp); }
    if (tiac&(1<<TOV0)) { irqSystem->SetIrqFlag(this, vectorTimer0Ovf); } else { irqSystem->ClearIrqFlag(vectorTimer0Ovf); }
    if (tiac&(1<<OCF1A)) { irqSystem->SetIrqFlag(this, vectorTimer1CompA); } else { irqSystem->ClearIrqFlag(vectorTimer1CompA); }
    if (tiac&(1<<OCF1B)) { irqSystem->SetIrqFlag(this, vectorTimer1CompB); } else { irqSystem->ClearIrqFlag(vectorTimer1CompB); }
    if (etiac&(1<<OCF1C)) { irqSystem->SetIrqFlag(this, vectorTimer1CompC); } else { irqSystem->ClearIrqFlag(vectorTimer1CompC); }
    if (tiac&(1<<ICF1)) { irqSystem->SetIrqFlag(this, vectorTimer1Capt); } else { irqSystem->ClearIrqFlag(vectorTimer1Capt); }
    if (tiac&(1<<TOV1)) { irqSystem->SetIrqFlag(this, vectorTimer1Ovf); } else { irqSystem->ClearIrqFlag(vectorTimer1Ovf); }
    if (tiac&(1<<OCF2)) { irqSystem->SetIrqFlag(this, vectorTimer2Comp); } else { irqSystem->ClearIrqFlag(vectorTimer2Comp); }
    if (tiac&(1<<TOV2)) { irqSystem->SetIrqFlag(this, vectorTimer2Ovf); } else { irqSystem->ClearIrqFlag(vectorTimer2Ovf); }
    if (etiac&(1<<OCF3A)) { irqSystem->SetIrqFlag(this, vectorTimer3CompA); } else { irqSystem->ClearIrqFlag(vectorTimer3CompA); }
    if (etiac&(1<<OCF3B)) { irqSystem->SetIrqFlag(this, vectorTimer3CompB); } else { irqSystem->ClearIrqFlag(vectorTimer3CompB); }
    if (etiac&(1<<OCF3C)) { irqSystem->SetIrqFlag(this, vectorTimer3CompC); } else { irqSystem->ClearIrqFlag(vectorTimer3CompC); }
    if (etiac&(1<<ICF3)) { irqSystem->SetIrqFlag(this, vectorTimer3Capt); } else { irqSystem->ClearIrqFlag(vectorTimer3Capt); }
    if (etiac&(1<<TOV3)) { irqSystem->SetIrqFlag(this, vectorTimer3Ovf); } else { irqSystem->ClearIrqFlag(vectorTimer3Ovf); }
}

#if 0
bool HWMegaTimer0123Irq::IsIrqFlagSet(unsigned int vector) {
    //XXX remove that function later!
    return 1;
    /*

    unsigned char tiac= timsk&tifr;
    unsigned char etiac= etimsk&etifr;

    if ((vector == vectorTimer0Comp ) && (tiac&(1<<OCF0))) { return 1; }
    if ((vector == vectorTimer0Ovf) && (tiac&(1<<TOV0))) { return 1; }
    if ((vector == vectorTimer1CompA) && (tiac&(1<<OCF1A))) { return 1; }
    if ((vector == vectorTimer1CompB) && (tiac&(1<<OCF1B))) { return 1; }
    if ((vector == vectorTimer1CompC) && (etiac&(1<<OCF1C))) { return 1; }
    if ((vector == vectorTimer1Capt) && (tiac&(1<<ICF1))) { return 1; }
    if ((vector == vectorTimer1Ovf) && (tiac&(1<<TOV1))) { return 1; }
    if ((vector == vectorTimer2Comp) && (tiac&(1<<OCF2))) { return 1; }
    if ((vector == vectorTimer2Ovf) && (tiac&(1<<TOV2))) { return 1; }
    if ((vector == vectorTimer3CompA) && (etiac&(1<<OCF3A))) { return 1; }
    if ((vector == vectorTimer3CompB) && (etiac&(1<<OCF3B))) { return 1; }
    if ((vector == vectorTimer3CompC) && (etiac&(1<<OCF3C))) { return 1; }
    if ((vector == vectorTimer3Capt) && (etiac&(1<<ICF3))) { return 1; }
    if ((vector == vectorTimer3Ovf) && (etiac&(1<<TOV3))) { return 1; }
    return 0;
    */
}
#endif

void HWMegaTimer0123Irq::ClearIrqFlag(unsigned int vector) {
    if (vector == vectorTimer0Comp ) {tifr&=0xff-(1<<OCF0);irqSystem->ClearIrqFlag(vectorTimer0Comp);}
    if (vector == vectorTimer0Ovf) {tifr&=0xff-(1<<TOV0);irqSystem->ClearIrqFlag(vectorTimer0Ovf);}
    if (vector == vectorTimer1CompA) {tifr&=0xff-(1<<OCF1A);irqSystem->ClearIrqFlag(vectorTimer1CompA);}
    if (vector == vectorTimer1CompB) {tifr&=0xff-(1<<OCF1B);irqSystem->ClearIrqFlag(vectorTimer1CompB);}
    if (vector == vectorTimer1CompC) {etifr&=0xff-(1<<OCF1C);irqSystem->ClearIrqFlag(vectorTimer1CompC);}
    if (vector == vectorTimer1Capt) {tifr&=0xff-(1<<ICF1);irqSystem->ClearIrqFlag(vectorTimer1Capt);}
    if (vector == vectorTimer1Ovf) {tifr&=0xff-(1<<TOV1);irqSystem->ClearIrqFlag(vectorTimer1Ovf);}
    if (vector == vectorTimer2Comp) {tifr&=0xff-(1<<OCF2);irqSystem->ClearIrqFlag(vectorTimer2Comp);}
    if (vector == vectorTimer2Ovf) {tifr&=0xff-(1<<TOV2);irqSystem->ClearIrqFlag(vectorTimer2Ovf);}
    if (vector == vectorTimer3CompA) {etifr&=0xff-(1<<OCF3A);irqSystem->ClearIrqFlag(vectorTimer3CompA);}
    if (vector == vectorTimer3CompB) {etifr&=0xff-(1<<OCF3B);irqSystem->ClearIrqFlag(vectorTimer3CompB);}
    if (vector == vectorTimer3CompC) {etifr&=0xff-(1<<OCF3C);irqSystem->ClearIrqFlag(vectorTimer3CompC);}
    if (vector == vectorTimer3Capt) {etifr&=0xff-(1<<ICF3);irqSystem->ClearIrqFlag(vectorTimer3Capt);}
    if (vector == vectorTimer3Ovf) {etifr&=0xff-(1<<TOV3);irqSystem->ClearIrqFlag(vectorTimer3Ovf);}
}

unsigned char HWMegaTimer0123Irq::GetTimsk() { return timsk;}
unsigned char HWMegaTimer0123Irq::GetTifr() {return tifr;}
unsigned char HWMegaTimer0123Irq::GetEtimsk() { return etimsk;}
unsigned char HWMegaTimer0123Irq::GetEtifr() {return etifr;}

void HWMegaTimer0123Irq::SetTimsk(unsigned char val) {
    timsk=val;
    CheckForIrq();
}

void HWMegaTimer0123Irq::SetTifr(unsigned char val) { 
    tifr&=~val;
    CheckForIrq();
}

void HWMegaTimer0123Irq::SetEtimsk(unsigned char val) { 
    etimsk=val;
    CheckForIrq();
}

void HWMegaTimer0123Irq::SetEtifr(unsigned char val) { 
    etifr&=~val; 
    CheckForIrq();
}




