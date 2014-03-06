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

#include "hwacomp.h"

#include "irqsystem.h"
#include "hwad.h"
#include "hwtimer.h"

HWAcomp::HWAcomp(AvrDevice *core,
                 HWIrqSystem *irqsys,
                 PinAtPort ain0,
                 PinAtPort ain1,
                 unsigned int _irqVec,
                 HWAd *_ad,
                 BasicTimerUnit *_timerA,
                 IOSpecialReg *_sfior,
                 BasicTimerUnit *_timerB,
                 bool _useBG):
    Hardware(core),
    TraceValueRegister(core, "ACOMP"),
    irqSystem(irqsys),
    pinAin0(ain0),
    pinAin1(ain1),
    useBG(_useBG),
    acme_sfior(false),
    enabled(true),
    irqVec(_irqVec),
    timerA(_timerA),
    timerB(_timerB),
    ad(_ad),
    sfior(_sfior),
    acsr_reg(this, "ACSR", this, &HWAcomp::GetAcsr, &HWAcomp::SetAcsr)
{
    // just check right assignment of IRQ vector number
    irqSystem->DebugVerifyInterruptVector(irqVec, this);
    // register callback for pin changes
    ain0.GetPin().RegisterCallback(this);
    ain1.GetPin().RegisterCallback(this);
    // vcc voltage and bandgap reference voltage
    v_cc = &(core->v_supply);
    v_bg = &(core->v_bandgap);
    // register to timer for input capture source, if available
    if(timerA != NULL)
        timerA->RegisterACompForICapture(this);
    if(timerB != NULL)
        timerB->RegisterACompForICapture(this);
    // register to ad to get signal changes from ADC multiplexer
    if(ad != NULL)
        ad->RegisterNotifyClient(this);
    // register for SFIOR, if set
    if(sfior != NULL)
        sfior->connectSRegClient(this);

    Reset();
}

HWAcomp::~HWAcomp() {
    if(ad != NULL)
        ad->UnregisterNotifyClient();
}

void HWAcomp::Reset() {
    acsr = 0;
    enabled = true;
    acme_sfior = false; // this assumes, that SFIOR register is also reseted
    if(GetIn0() > GetIn1())
        acsr |= ACO;
}

void HWAcomp::SetAcsr(unsigned char val) {
    unsigned char old = acsr & (ACO|ACI);
    bool old_acic = (acsr & ACIC) == ACIC;
    bool old_bg = (acsr & ACBG) == ACBG;
    if(!useBG)
        val &= ~ACBG; // delete ACBG bit, if not available
    // store data
    acsr = val & ~(ACO|ACI); // mask out new bits
    acsr |= old; // and restore old bits
    // if AIN0 source is changed, calculate comparator state
    bool bg = (acsr & ACBG) == ACBG;
    if(old_bg != bg)
        PinStateHasChanged(NULL);
    if(val & ACI)
        acsr &= ~ACI; // reset ACI if ACI in val is set to 1
    enabled = (acsr & ACD) == 0; // disabled, if ACD is 1!
    // reflect ACIC state to timer, if available
    bool acic = (acsr & ACIC) == ACIC;
    if(acic != old_acic) {
        if(timerA != NULL)
            timerA->SetACIC(acic);
        if(timerB != NULL)
            timerB->SetACIC(acic);
    }
    // if interrupt enabled and ACI is asserted, then fire interrupt
    if(enabled) {
        if((acsr & ( ACI|ACIE)) == (ACI|ACIE))
            irqSystem->SetIrqFlag(this, irqVec);
        else
            irqSystem->ClearIrqFlag(irqVec);
    }
}

float HWAcomp::GetIn0(void) {
    if(useBG && ((acsr & ACBG) == ACBG))
        return v_bg->GetRawAnalog();
    else
        return pinAin0.GetAnalogValue(v_cc->GetRawAnalog());
}

float HWAcomp::GetIn1(void) {
    float vcc = v_cc->GetRawAnalog();
    if(isSetACME())
        return ad->GetADMuxValue(vcc);
    else
        return pinAin1.GetAnalogValue(vcc);
}

void HWAcomp::PinStateHasChanged(Pin *p) {
    // get old comparator state and IRQ mode
    bool old = (acsr & ACO);
    unsigned char irqmode = acsr & (ACIS1|ACIS0);
    
    // if unit is disabled (to save power), do not check state
    if(!enabled)
        return;

    // calculate state
    if(GetIn0() > GetIn1()) {
        // set output to high
        if(old == false) {
            // rising edge
            acsr |= ACO;
            // fire IRQ, if necessary
            if((irqmode == 0) || (irqmode == (ACIS1|ACIS0))) {
                acsr |= ACI;
                if(acsr & ACIE) irqSystem->SetIrqFlag(this, irqVec);
            }
        }
    } else {
        // set output to low
        if (old == true) {
            // falling edge
            acsr &= ~ACO;
            // fire IRQ, if necessary
            if((irqmode == 0) || (irqmode == ACIS1)) {
                acsr |= ACI;
                if(acsr & ACIE) irqSystem->SetIrqFlag(this, irqVec);
            }
        }
    }
}

void HWAcomp::NotifySignalChanged(void) {
    // just start notify functionality
    if(isSetACME())
        PinStateHasChanged(NULL);
}

void HWAcomp::ClearIrqFlag(unsigned int vector){
    if (vector == irqVec) {
        acsr &= ~ACI;
        irqSystem->ClearIrqFlag(irqVec);
    }
}

unsigned char HWAcomp::set_from_reg(const IOSpecialReg* reg, unsigned char nv) {
    // check, if ACME bit is set
    acme_sfior = (nv & 0x08) == 0x08;
    // check comparator state
    PinStateHasChanged(NULL);
    return nv;
}

bool HWAcomp::isSetACME(void) {
    // ACME feature is only available, if a ADC exists and ADC isn't enabled!
    if(ad != NULL && !ad->IsADEnabled()) {
        // check from SFIOR register, if available
        if(sfior != NULL)
            return acme_sfior;
        // or check from ADC unit (there in ADCSRB register)
        return ad->IsSetACME();
    } else
        // otherwise ACME isn't available or not enabled
        return false;
}
