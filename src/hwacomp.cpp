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

#include "hwacomp.h"

#include "irqsystem.h"

#define ACD 0x80
#define ACO 0x20
#define ACI 0x10
#define ACIE 0x08
#define ACIC 0x04
#define ACIS1 0x02
#define ACIS0 0x01


HWAcomp::HWAcomp(AvrDevice *core, HWIrqSystem *irqsys, PinAtPort ain0, PinAtPort ain1, unsigned int _irqVec):
    Hardware(core),
    TraceValueRegister(core, "ACOMP"),
    irqSystem(irqsys),
    pinAin0(ain0), pinAin1(ain1),
    acsr_reg(this, "ACSR", this, &HWAcomp::GetAcsr, &HWAcomp::SetAcsr),
    irqVec(_irqVec)
{
    irqSystem->DebugVerifyInterruptVector(irqVec, this);
    ain0.GetPin().RegisterCallback(this);
    ain1.GetPin().RegisterCallback(this);
    Reset();
}


void HWAcomp::Reset(){
    acsr=0;
}

void HWAcomp::SetAcsr(unsigned char val) {
    unsigned char old= acsr&0x30; 
    acsr=val&0x9f; //Bits 5 & 6 are read only for 4433 that is not ok TODO XXX
    acsr|= old;
    if (val & ACI) {
        acsr &=~ACI; // reset ACI if ACI in val is set 1
    }

    if ( (acsr & ( ACI | ACIE) ) == ( ACI | ACIE) ) {
        irqSystem->SetIrqFlag(this, irqVec);
    } else {
        irqSystem->ClearIrqFlag(irqVec);
    }

}

unsigned char HWAcomp::GetAcsr() {
    return acsr;
}

unsigned int HWAcomp::CpuCycle() {
    return 0;
}

void HWAcomp::PinStateHasChanged(Pin *p) {
    bool oldComp=(acsr & ACO);

    
    if (pinAin0.GetAnalog()>pinAin1.GetAnalog()) { //set comperator 1
        if (oldComp==false) { //not set before
            acsr|=ACO;
            //do the irq TODO
            unsigned char irqMask= acsr & (ACIS1|ACIS0);
            if ( (irqMask==0) || (irqMask==( ACIS1 | ACIS0) ) ) { //toggle or rising
                acsr|=ACI;
                if (acsr&ACIE) irqSystem->SetIrqFlag(this, irqVec);
            }
        }
    } else { //set comperator 0
        if (oldComp==true) { //ACO was set before
            acsr&=~ACO;
            //do the irq
            unsigned char irqMask= acsr & (ACIS1|ACIS0);
            if ( (irqMask==0) || (irqMask==( ACIS1 ) )) { //toggle or falling
                acsr|=ACI;
                if (acsr&ACIE) irqSystem->SetIrqFlag(this, irqVec);
            }
        }
    }

}

void HWAcomp::ClearIrqFlag(unsigned int vector){
    if (vector==irqVec) {
        acsr&=~ACI;
        irqSystem->ClearIrqFlag(irqVec);
    }
}

