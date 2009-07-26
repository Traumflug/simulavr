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

#include "hardware.h"
#include "avrdevice.h"
#include "timerprescaler.h"
#include "hwtimer.h"
#include "hwtimer01irq.h"
#include "trace.h"
#include "helper.h"

using namespace std;

void HWTimer0::TimerCompareAfterCount() {
    if ((tcnt==1) ) { //overflow occured! when leaving 0
        timer01irq->AddFlagToTifr(timer01irq->TOV0);    //set TOV0 in TIFR
    }
}

HWTimer0::HWTimer0(AvrDevice *c, HWPrescaler *p, HWTimer01Irq *s,
                   PinAtPort pi, int n):
    Hardware(c),core(c),pin_t0(pi),
    tccr_reg(core, "TIMER"+int2str(n)+".TCCR",
             this, &HWTimer0::GetTccr, &HWTimer0::SetTccr),
    tcnt_reg(core, "TIMER"+int2str(n)+".TCNT",
             this, &HWTimer0::GetTcnt, &HWTimer0::SetTccr) {
    //core->AddToCycleList(this);
    prescaler=p;
    timer01irq= s;
    Reset();
}

void HWTimer0::SetTccr(unsigned char val) {
    tccr=val; 

    if ( tccr & 0x07) {
        core->AddToCycleList(this);
    } else {
        core->RemoveFromCycleList(this);
    }
}

unsigned int HWTimer0::CpuCycle(){
    switch (tccr) {
        case STOP:
            break;

        case CK:
            tcnt++;
            TimerCompareAfterCount();
            break;

        case CK8:
            if ((prescaler->GetValue()%8)==0){
                tcnt++;
                TimerCompareAfterCount();
            }
            break;

        case CK64:
            if ((prescaler->GetValue()%64)==0){
                tcnt++;
                TimerCompareAfterCount();
            }
            break;

        case CK256:
            if ((prescaler->GetValue()%256)==0) {
                tcnt++;
                TimerCompareAfterCount();
            }
            break;

        case CK1024:
            if ((prescaler->GetValue()%1024)==0) {
                tcnt++;
                TimerCompareAfterCount();
            }
            break;

        case T0_FALLING:
            if ((pin_t0==0) && (t0_old ==1)){
                t0_old=0;
                tcnt++;
                TimerCompareAfterCount();
            }
            break;

        case T0_RISING:
            if ((pin_t0==1) && (t0_old ==0)){ 
                t0_old=1; 
                tcnt++;
                TimerCompareAfterCount();
            }
            break;

    }

    return 0;
}


HWTimer1::HWTimer1(AvrDevice *c, HWPrescaler *p, HWTimer01Irq *s,
                   PinAtPort t1, PinAtPort oca, PinAtPort ocb,
                   PinAtPort icp, int n):
    Hardware(c), core(c), pin_t1(t1), pin_oc1a(oca), pin_oc1b(ocb), pin_icp(icp),
    tccr1a_reg(core, "TIMER"+int2str(n)+".TCCR1A",
               this, &HWTimer1::GetTccr1a, &HWTimer1::SetTccr1a),
    tccr1b_reg(core, "TIMER"+int2str(n)+".TCCR1B",
               this, &HWTimer1::GetTccr1b, &HWTimer1::SetTccr1b),
    tcnt1h_reg(core, "TIMER"+int2str(n)+".TCNT1H",
               this, &HWTimer1::GetTcnt1h, &HWTimer1::SetTcnt1h),
    tcnt1l_reg(core, "TIMER"+int2str(n)+".TCNT1L",
               this, &HWTimer1::GetTcnt1l, &HWTimer1::SetTcnt1l),
    ocr1ah_reg(core, "TIMER"+int2str(n)+".OCR1AH",
               this, &HWTimer1::GetOcr1ah, &HWTimer1::SetOcr1ah),
    ocr1al_reg(core, "TIMER"+int2str(n)+".OCR1AL",
               this, &HWTimer1::GetOcr1al, &HWTimer1::SetOcr1al),
    ocr1bh_reg(core, "TIMER"+int2str(n)+".OCR1BH",
               this, &HWTimer1::GetOcr1bh, &HWTimer1::SetOcr1bh),
    ocr1bl_reg(core, "TIMER"+int2str(n)+".OCR1BL",
               this, &HWTimer1::GetOcr1bl, &HWTimer1::SetOcr1bl),
    icr1h_reg(core, "TIMER"+int2str(n)+".ICR1H",
              this, &HWTimer1::GetIcr1h, 0),
    icr1l_reg(core, "TIMER"+int2str(n)+".ICR1L",
              this, &HWTimer1::GetIcr1l, 0) { 
    //c->AddToCycleList(this);
    cntDir=1; //start with upcounting
    prescaler=p, 
    timer01irq=s;
    Reset();
}

//compares
void HWTimer1::TimerCompareAfterCount() {
    if ( (tccr1a & 0x03) != 0x00) { //timer is in pwm mode
        if ((tcnt1==0) && (cntDir==0)) { cntDir=1; } //count up now
        if ((tcnt1==topValue) && (cntDir==1)) { cntDir=0; } //count down now
    }


    if ((tcnt1==1) && (cntDir==1)) { //overflow occured! while leaving 0 upward
        timer01irq->AddFlagToTifr(timer01irq->TOV1);    //set TOV1 in TIFR
    }
    if (tcnt1==ocr1a) {
        if ((tccr1b&0x08)!=0) { //CTC1 is set -> compareA: reset counter
            tcnt1=0;
        }   

        if ( (tccr1a & 0x03) == 0x00) { //timer is not in pwm mode
            switch (tccr1a&(0x80|0x40)) {
                case 0x00:
                    ; //Nothing todo, settings menaged on other place :-)
                    break;

                case 0x40:
                    if (last_ocr1a==0) { last_ocr1a=1; } else { last_ocr1a=0;}
                    pin_oc1a.SetAlternatePort(last_ocr1a);  //Toggle Pin
                    break;

                case 0x80:
                    last_ocr1a=0;
                    pin_oc1a.SetAlternatePort(last_ocr1a);
                    break;

                case 0xc0:
                    last_ocr1a=1;
                    pin_oc1a.SetAlternatePort(last_ocr1a);
                    break;

            } // end of switch

        } else { // timer is in pwm mode

            switch ( tccr1a&(0x80|0x40) ) {
                case 0x00:
                case 0x40:
                    ; //not connected, handled at other place :-) where? :-(
                    break;

                case 0x80:
                    if (cntDir==1) { //clear on upcounting 
                        last_ocr1a=0;
                        pin_oc1a.SetAlternatePort(last_ocr1a);
                    } else {
                        last_ocr1a=1;
                        pin_oc1a.SetAlternatePort(last_ocr1a);
                    }

                    break;

                case 0xc0:
                    if (cntDir==1) { //set on upcounting
                        last_ocr1a=1;
                        pin_oc1a.SetAlternatePort(last_ocr1a);
                    } else {
                        last_ocr1a=0;
                        pin_oc1a.SetAlternatePort(last_ocr1a);
                    }
                    break;
            }


        }
        timer01irq->AddFlagToTifr(timer01irq->OCF1A);   //set OCF1A in TIFR

    } //end of compare

    if (tcnt1==ocr1b) {
        if ( (tccr1a & 0x03) == 0x00) { //timer is not in pwm mode
            switch(tccr1a&(0x30)) {
                case 0x00:
                    ; //Nothing todo, settings menaged on other place :-)
                    break;

                case 0x10:
                    if (last_ocr1b==0) { last_ocr1b=1; } else { last_ocr1b=0;}
                    pin_oc1b.SetAlternatePort(last_ocr1b);  //Toggle Pin
                    break;

                case 0x20:
                    last_ocr1b=0;
                    pin_oc1b.SetAlternatePort(last_ocr1b);
                    break;

                case 0x30:
                    last_ocr1b=1;
                    pin_oc1b.SetAlternatePort(last_ocr1b);
                    break;

            }       //end of switch
        } else { //timer is in pwm mode
            switch ( tccr1a&(0x30) ) {
                case 0x00:
                case 0x10:
                    ; //not connected, handled at other place :-) where? :-(
                    break;

                case 0x20:
                    if (cntDir==1) { //clear on upcounting 
                        last_ocr1b=0;
                        pin_oc1b.SetAlternatePort(last_ocr1b);
                    } else {
                        last_ocr1b=1;
                        pin_oc1b.SetAlternatePort(last_ocr1b);
                    }

                    break;

                case 0x30:
                    if (cntDir==1) { //set on upcounting
                        last_ocr1b=1;
                        pin_oc1b.SetAlternatePort(last_ocr1b);
                    } else {
                        last_ocr1b=0;
                        pin_oc1b.SetAlternatePort(last_ocr1b);
                    }
                    break;
            }

        }

        timer01irq->AddFlagToTifr(timer01irq->OCF1B);   //set OCF1B in TIFR
    } // end of compare
}

unsigned int HWTimer1::CpuCycle(){
    int addVal;

    if ( (tccr1a & 0x03) == 0x00 ) { //timer is not in pwm mode
        addVal=1;
    } else {
        if (cntDir==1) {
            addVal=1;
        } else {
            addVal=-1;
        }
    }

    switch (tccr1b&0x7) {
        case STOP:
            break;

        case CK:
            tcnt1+=addVal;
            TimerCompareAfterCount();
            break;

        case CK8:
            if ((prescaler->GetValue()%8)==0) {
                tcnt1+=addVal;
                TimerCompareAfterCount();
            }
            break;

        case CK64:
            if ((prescaler->GetValue()%64)==0) {
                tcnt1+=addVal;
                TimerCompareAfterCount();
            }
            break;

        case CK256:
            if ((prescaler->GetValue()%256)==0) {
                tcnt1+=addVal;
                TimerCompareAfterCount();
            }
            break;

        case CK1024:
            if ((prescaler->GetValue()%1024)==0) {
                tcnt1+=addVal;
                TimerCompareAfterCount();
            }
            break;

        case T0_FALLING:
            if ((pin_t1==0) && (t1_old ==1)){ 
                t1_old=0;
                tcnt1+=addVal;
                TimerCompareAfterCount();
            }
            break;

        case T0_RISING:
            if ((pin_t1==1) && (t1_old ==0)){
                t1_old=1;
                tcnt1+=addVal;
                TimerCompareAfterCount();
            }
            break;

    }

    //input capture register

    // edge detection
    if ((pin_icp != icp_old) && (pin_icp == ((tccr1b&0x40)==0x40)))
        inputCaptureNoiseCnt=1;
    // and noise cancelling
    else if (inputCaptureNoiseCnt && (pin_icp == ((tccr1b&0x40)==0x40)))
        inputCaptureNoiseCnt+=1;
    else
        inputCaptureNoiseCnt=0;

    if (inputCaptureNoiseCnt>3*((tccr1b&0x80)==0x80)) {
        // captured!!
        inputCaptureNoiseCnt=0;
        timer01irq->AddFlagToTifr(timer01irq->ICF1);    //set ICF1 in TIFR
        icr1=tcnt1; //Capture
    }
    
    icp_old=pin_icp;
    return 0;
}

void HWTimer1::SetTccr1b(unsigned char val) {
    tccr1b=val;
    if (tccr1b & 0x07) {
    core->AddToCycleList(this);
    } else {
    core->RemoveFromCycleList(this);
    }
}

void HWTimer1::SetTccr1a(unsigned char val) { 
    tccr1a=val; //Setting for alternate pin ocr1a could be changed so test for this feature
    if ((tccr1a & 0x03) == 0x00 ) { // not in pwm mode
        switch(tccr1a&0xc0) {
            case 0:
                pin_oc1a.SetUseAlternatePortIfDdrSet(0); 
                break;

            case 0x40:
            case 0x80:
            case 0xc0:
                pin_oc1a.SetUseAlternatePortIfDdrSet(1);    
                pin_oc1a.SetAlternatePort(last_ocr1a);
                break;
        }

        switch(tccr1a&0x30) {
            case 0:
                pin_oc1b.SetUseAlternatePort(0); 
                pin_oc1b.SetDdr(0);
                break;

            case 0x10:
            case 0x20:
            case 0x30:
                pin_oc1b.SetUseAlternatePort(1);    
                pin_oc1b.SetDdr(1); 
                pin_oc1b.SetAlternatePort(last_ocr1b);
                break;
        } // end of switch
    } else { // timer is in pwm mode
        switch (tccr1a&0x03) {
            case 0x01:
                topValue=0xff;
                break;

            case 0x02:
                topValue=0x1ff;
                break;

            case 0x03:
                topValue=0x3ff;
                break;
        }

        switch(tccr1a&0xc0) {
            case 0x00:
            case 0x40:
                pin_oc1a.SetUseAlternatePortIfDdrSet(0); 
                break;

            case 0x80:
            case 0xc0:
                pin_oc1a.SetUseAlternatePortIfDdrSet(1);    
                pin_oc1a.SetAlternatePort(last_ocr1a);
                break;
        }

        switch(tccr1a&0x30) {
            case 0x00:
            case 0x10:
                pin_oc1b.SetUseAlternatePort(0); 
                pin_oc1b.SetDdr(0);
                break;

            case 0x20:
            case 0x30:
                pin_oc1b.SetUseAlternatePort(1);    
                pin_oc1b.SetDdr(1); 
                pin_oc1b.SetAlternatePort(last_ocr1b);
                break;
        }
    }

}

/////////////////////////////////////////////////////////////////////////

BasicTimerUnit::BasicTimerUnit(AvrDevice *core,
                               PrescalerMultiplexer *p,
                               int unit,
                               IRQLine* tov,
                               IRQLine* tcap,
                               int countersize):
    Hardware(core),
    TraceValueRegister(core, "TIMER" + int2str(unit)),
    core(core),
    premx(p),
    timerOverflow(tov),
    timerCapture(tcap)
{
    // check counter size and set limit_max
    if(countersize != 8 && countersize != 16)
        avr_error("wrong parameter: countersize=%d", countersize);
    if(countersize == 8)
        limit_max = 0xff;
    else
        limit_max = 0xffff;
    
    // set input capture register
    icapRegister = 0;
    
    // create TraceValue for counter itself
    counterTrace = new TraceValue(countersize, GetTraceValuePrefix() + "Counter");
    RegisterTraceValue(counterTrace);
    counterTrace->set_written(0);
    core->dump_manager->regTrace(counterTrace);
    
    // disable all compare registers, output pins and reset Compare IRQ's
    for(int i = 0; i < OCRIDX_maxUnits; i++) {
        compareEnable[i] = false;
        timerCompare[i] = NULL;
        compare_output[i] = NULL;
    }
    
    // set wgm functions
    for(int i = 0; i < WGM_tablesize; i++)
        wgmfunc[i] = &BasicTimerUnit::WGMFunc_noop;
    
    // reset internal values
    Reset();
    
}

BasicTimerUnit::~BasicTimerUnit() {
    delete counterTrace;
}

void BasicTimerUnit::CountTimer() {
    vlast_tcnt = vtcnt; // save cycle - 1 counter value
    if(updown_counting) {
        // first handle compare events
        if(compareEnable[0]) {
            if(vlast_tcnt == compare[0])
                HandleEvent(EVT_COMPARE_1);
            if(compareEnable[1]) {
                if(vlast_tcnt == compare[1])
                    HandleEvent(EVT_COMPARE_2);
                if(compareEnable[2]) {
                    if(vlast_tcnt == compare[2])
                        HandleEvent(EVT_COMPARE_3);
                }
            }
        }

        // then handle overflow events
        if(vlast_tcnt == limit_bottom)
            HandleEvent(EVT_BOTTOM_REACHED);
        else if(vlast_tcnt == limit_top)
            HandleEvent(EVT_TOP_REACHED);

        // counter counts up and down
        if(count_down) {
            vtcnt--;
            counterTrace->change(vtcnt);
            if(vtcnt == limit_bottom)
                count_down = false; // now count up
        } else {
            vtcnt++;
            counterTrace->change(vtcnt);
            if(vtcnt == limit_top)
                count_down = true; // now count down
        }
    } else {
        // first simple up counting till overflow (used in normal mode)
        vtcnt++;
        if(vtcnt > limit_max) { // overflow?
            HandleEvent(EVT_MAX_REACHED);
            vtcnt &= limit_max; // reset overflow
        }
        
        // handle bottom event
        if(vlast_tcnt == limit_bottom)
            HandleEvent(EVT_BOTTOM_REACHED);
        
        // handle top event
        if(vlast_tcnt == limit_top)
            HandleEvent(EVT_TOP_REACHED);
        
        // handle compare events (to fire interrupts or change counter
        if(compareEnable[0]) {
            if(vlast_tcnt == compare[0])
                HandleEvent(EVT_COMPARE_1);
            if(compareEnable[1]) {
                if(vlast_tcnt == compare[1])
                    HandleEvent(EVT_COMPARE_2);
                if(compareEnable[2]) {
                    if(vlast_tcnt == compare[2])
                        HandleEvent(EVT_COMPARE_3);
                }
            }
        }
        
        // trace the counter change
        counterTrace->change(vtcnt);
    }
}

void BasicTimerUnit::SetClockMode(int mode) {
    cs = mode;
    if(cs != 0) {
        core->AddToCycleList(this);
    } else {
        core->RemoveFromCycleList(this);
    }
}

void BasicTimerUnit::SetCounter(unsigned long val) {
    vtcnt = val;
    vlast_tcnt = 0x10000; // set to a invalid value!
    counterTrace->change(val);
}

void BasicTimerUnit::SetCompareOutputMode(int idx, COMtype mode) {
    com[idx] = mode;
    if(compare_output[idx]) {
        if(mode == COM_NOOP)
            compare_output[idx]->SetUseAlternatePortIfDdrSet(false);
        else {
            compare_output[idx]->SetUseAlternatePortIfDdrSet(true);
            compare_output[idx]->SetAlternatePort(compare_output_state[idx]);
        }
    }
}

void BasicTimerUnit::SetCompareOutput(int idx) {
    COMtype mode = com[idx];
    bool new_state, old_state = compare_output_state[idx];
    switch(mode) {
        case COM_NOOP:
            return;
            
        case COM_TOGGLE:
            new_state = old_state ? false : true;
            break;
            
        case COM_CLEAR:
            new_state = false;
            break;
            
        case COM_SET:
            new_state = true;
            break;
    }
    compare_output_state[idx] = new_state;
    if(compare_output[idx] && old_state != new_state)
        compare_output[idx]->SetAlternatePort(new_state);
}

void BasicTimerUnit::SetPWMCompareOutput(int idx, bool top) {
    COMtype mode = com[idx];
    bool new_state, old_state = compare_output_state[idx];
    switch(mode) {
        case COM_NOOP:
            return;
            
        case COM_TOGGLE:
            if(wgm == WGM_FASTPWM_OCRA && idx == OCRIDX_A)
                // special mode in case of WGM_FASTPWM_OCRA!
                SetCompareOutput(OCRIDX_A);
            else
                avr_warning("COM==1 in PWM mode is reserved!");
            break;
            
        case COM_CLEAR:
            if(top)
                new_state = true;
            else
                new_state = false;
            break;
            
        case COM_SET:
            if(top)
                new_state = false;
            else
                new_state = true;
            break;
    }
    compare_output_state[idx] = new_state;
    if(compare_output[idx] && old_state != new_state)
        compare_output[idx]->SetAlternatePort(new_state);
}

void BasicTimerUnit::WGMfunc_normal(CEtype event) {
    switch(event) {
        case EVT_MAX_REACHED:
            timerOverflow->fireInterrupt();
            break;
            
        case EVT_COMPARE_1:
            if(timerCompare[0]) {
                timerCompare[0]->fireInterrupt();
                SetCompareOutput(0);
            }
            break;
            
        case EVT_COMPARE_2:
            if(timerCompare[1]) {
                timerCompare[1]->fireInterrupt();
                SetCompareOutput(1);
            }
            break;
            
        case EVT_COMPARE_3:
            if(timerCompare[2]) {
                timerCompare[2]->fireInterrupt();
                SetCompareOutput(2);
            }
            break;
            
        default:
            break;
    }
}

void BasicTimerUnit::WGMfunc_ctc(CEtype event) {
    switch(event) {
        case EVT_MAX_REACHED:
            timerOverflow->fireInterrupt();
            break;
            
        case EVT_TOP_REACHED:
            vtcnt = limit_bottom;
            break;
            
        case EVT_COMPARE_1:
            if(timerCompare[0]) {
                timerCompare[0]->fireInterrupt();
                SetCompareOutput(0);
            }
            break;
            
        case EVT_COMPARE_2:
            if(timerCompare[1]) {
                timerCompare[1]->fireInterrupt();
                SetCompareOutput(1);
            }
            break;
            
        case EVT_COMPARE_3:
            if(timerCompare[2]) {
                timerCompare[2]->fireInterrupt();
                SetCompareOutput(2);
            }
            break;
            
        default:
            break;
    }
}

void BasicTimerUnit::WGMfunc_fastpwm(CEtype event) {
    switch(event) {
        case EVT_TOP_REACHED:
            // fire overflow interrupt
            timerOverflow->fireInterrupt();
            // if ICR or OCRA mode, fire too this interrupts
            if(wgm == WGM_FASTPWM_OCRA) {
                if(timerCompare[OCRIDX_A])
                    timerCompare[OCRIDX_A]->fireInterrupt();
            } else if(wgm == WGM_FASTPWM_ICR) {
                if(timerCapture)
                    timerCapture->fireInterrupt();
            }
            // process compare output unit
            for(int i; i < OCRIDX_maxUnits; i++)
                SetPWMCompareOutput(i, true);
            // reset counter
            vtcnt = limit_bottom;
            break;
            
        case EVT_BOTTOM_REACHED:
            // update OC registers
            for(int i; i < OCRIDX_maxUnits; i++) {
                if(i == OCRIDX_A) {
                    if(wgm == WGM_FASTPWM_8BIT)
                        // mask to 0xff
                        compare[i] = compare_dbl[i] & 0xff;
                    else if(wgm == WGM_FASTPWM_9BIT)
                        // mask to 0x1ff
                        compare[i] = compare_dbl[i] & 0x1ff;
                    else if(wgm == WGM_FASTPWM_10BIT)
                        // mask to 0x3ff
                        compare[i] = compare_dbl[i] & 0x3ff;
                    else if(wgm == WGM_FASTPWM_OCRA)
                        // set new top value
                        limit_top = compare_dbl[i];
                } else
                    compare[i] = compare_dbl[i];
            }
            break;
            
        case EVT_COMPARE_1:
            if(timerCompare[0] && wgm != WGM_FASTPWM_OCRA) {
                timerCompare[0]->fireInterrupt();
                SetPWMCompareOutput(0, false);
            }
            break;
            
        case EVT_COMPARE_2:
            if(timerCompare[1]) {
                timerCompare[1]->fireInterrupt();
                SetPWMCompareOutput(1, false);
            }
            break;
            
        case EVT_COMPARE_3:
            if(timerCompare[2]) {
                timerCompare[2]->fireInterrupt();
                SetPWMCompareOutput(2, false);
            }
            break;
            
        default:
            break;
    }
}

void BasicTimerUnit::Reset() {
    vtcnt = 0;
    limit_bottom = 0;
    limit_top = limit_max;
    vlast_tcnt = limit_top;
    for(int i = 0; i < OCRIDX_maxUnits; i++) {
        compare_dbl[i] = 0;
        compare[i] = 0;
        SetCompareOutputMode(i, COM_NOOP);
        compare_output_state[i] = false;
    }
    SetClockMode(0);
    updown_counting = false;
    count_down = false;
    wgm = WGM_NORMAL;
}

unsigned int BasicTimerUnit::CpuCycle() {
    if(premx->isClock(cs))
        CountTimer();
    InputCapture();
    return 0;
}

HWTimer8::HWTimer8(AvrDevice *core,
                   PrescalerMultiplexer *p,
                   int unit,
                   IRQLine* tov,
                   IRQLine* tcompA,
                   PinAtPort* outA,
                   IRQLine* tcompB,
                   PinAtPort* outB):
    BasicTimerUnit(core, p, unit, tov, NULL, 8),
    tcnt_reg(core, GetTraceValuePrefix() + "TCNT",
             this, &HWTimer8::Get_TCNT, &HWTimer8::Set_TCNT),
    ocra_reg(core, GetTraceValuePrefix() + "OCRA",
             this, &HWTimer8::Get_OCRA, &HWTimer8::Set_OCRA),
    ocrb_reg(core, GetTraceValuePrefix() + "OCRB",
             this, &HWTimer8::Get_OCRB, &HWTimer8::Set_OCRB)
{
    // enable OC units and disable registers
    if(tcompA) {
        compareEnable[0] = true;
        timerCompare[0] = tcompA;
        compare_output[0] = outA;
    }
    if(tcompB) {
        compareEnable[1] = true;
        timerCompare[1] = tcompB;
        compare_output[1] = outB;
    }
    
    // set WGM table
    wgmfunc[WGM_NORMAL] = &HWTimer8::WGMfunc_normal;
    wgmfunc[WGM_CTC_OCRA] = &HWTimer8::WGMfunc_ctc;
    wgmfunc[WGM_FASTPWM_8BIT] = &HWTimer8::WGMfunc_fastpwm;
    
    // reset unit
    Reset();
}

void HWTimer8::Reset(void) {
    BasicTimerUnit::Reset();
}

void HWTimer8::ChangeWGM(WGMtype mode) {
    wgm = mode;
    switch(wgm) {
        case WGM_NORMAL:
            limit_top = limit_max;
            break;
            
        case WGM_CTC_OCRA:
            limit_top = compare[0];
            break;
            
        case WGM_FASTPWM_8BIT:
            limit_top = limit_max;
            break;
    }
}

void HWTimer8::SetCompareRegister(int idx, unsigned char val) {
    if(WGMisPWM())
        compare_dbl[idx] = val;
    else {
        compare[idx] = val;
        compare_dbl[idx] = val;
        if(wgm == WGM_CTC_OCRA && idx == 0)
            // in case od setting OCRA and WGM mode is CTC with OCRA, then set
            // also counter top value here
            limit_top = val;
    }
}

unsigned char HWTimer8::GetCompareRegister(int idx) {
    if(WGMisPWM())
        return compare_dbl[idx] & 0xff;
    else
        return compare[idx] & 0xff;
}

HWTimer16::HWTimer16(AvrDevice *core,
                     PrescalerMultiplexer *p,
                     int unit,
                     IRQLine* tov,
                     IRQLine* tcompA,
                     PinAtPort* outA,
                     IRQLine* tcompB,
                     PinAtPort* outB,
                     IRQLine* tcompC,
                     PinAtPort* outC,
                     IRQLine* ticap):
    BasicTimerUnit(core, p, unit, tov, ticap, 16),
    tcnt_h_reg(core, GetTraceValuePrefix() + "TCNTH",
               this, &HWTimer16::Get_TCNTH, &HWTimer16::Set_TCNTH),
    tcnt_l_reg(core, GetTraceValuePrefix() + "TCNTL",
               this, &HWTimer16::Get_TCNTL, &HWTimer16::Set_TCNTL),
    ocra_h_reg(core, GetTraceValuePrefix() + "OCRAH",
               this, &HWTimer16::Get_OCRAH, &HWTimer16::Set_OCRAH),
    ocra_l_reg(core, GetTraceValuePrefix() + "OCRAL",
               this, &HWTimer16::Get_OCRAL, &HWTimer16::Set_OCRAL),
    ocrb_h_reg(core, GetTraceValuePrefix() + "OCRBH",
               this, &HWTimer16::Get_OCRBH, &HWTimer16::Set_OCRBH),
    ocrb_l_reg(core, GetTraceValuePrefix() + "OCRBL",
               this, &HWTimer16::Get_OCRBL, &HWTimer16::Set_OCRBL),
    ocrc_h_reg(core, GetTraceValuePrefix() + "OCRCH",
               this, &HWTimer16::Get_OCRCH, &HWTimer16::Set_OCRCH),
    ocrc_l_reg(core, GetTraceValuePrefix() + "OCRCL",
               this, &HWTimer16::Get_OCRCL, &HWTimer16::Set_OCRCL),
    icr_h_reg(core, GetTraceValuePrefix() + "ICRH",
              this, &HWTimer16::Get_ICRH, &HWTimer16::Set_ICRH),
    icr_l_reg(core, GetTraceValuePrefix() + "ICRL",
              this, &HWTimer16::Get_ICRL, &HWTimer16::Set_ICRL)
{
    // enable OC units and disable registers
    if(tcompA) {
        compareEnable[0] = true;
        timerCompare[0] = tcompA;
        compare_output[0] = outA;
    }
    if(tcompB) {
        compareEnable[1] = true;
        timerCompare[1] = tcompB;
        compare_output[1] = outB;
    }
    if(tcompC) {
        compareEnable[2] = true;
        timerCompare[2] = tcompC;
        compare_output[2] = outC;
    }
    
    // set WGM table
    wgmfunc[WGM_NORMAL] = &HWTimer16::WGMfunc_normal;
    wgmfunc[WGM_CTC_OCRA] = &HWTimer16::WGMfunc_ctc;
    wgmfunc[WGM_CTC_ICR] = &HWTimer16::WGMfunc_ctc;
    wgmfunc[WGM_FASTPWM_8BIT] = &HWTimer16::WGMfunc_fastpwm;
    wgmfunc[WGM_FASTPWM_9BIT] = &HWTimer16::WGMfunc_fastpwm;
    wgmfunc[WGM_FASTPWM_10BIT] = &HWTimer16::WGMfunc_fastpwm;
    wgmfunc[WGM_FASTPWM_OCRA] = &HWTimer16::WGMfunc_fastpwm;
    wgmfunc[WGM_FASTPWM_ICR] = &HWTimer16::WGMfunc_fastpwm;
    
    // reset unit
    Reset();
}

void HWTimer16::Reset(void) {
    BasicTimerUnit::Reset();
    // initialize temp. register for 16Bit access
    accessTempRegister = 0;
}

void HWTimer16::SetCompareRegister(int idx, bool high, unsigned char val) {
    unsigned long temp;
    if(high) {
        accessTempRegister = val;
    } else {
        temp = (accessTempRegister << 8) + val;
        if(WGMisPWM())
            compare_dbl[idx] = temp;
        else {
            compare[idx] = temp;
            compare_dbl[idx] = temp;
            if(wgm == WGM_CTC_OCRA && idx == 0)
                // in case od setting OCRA and WGM mode is CTC with OCRA, then set
                // also counter top value here
                limit_top = temp;
        }
    }
}

unsigned char HWTimer16::GetCompareRegister(int idx, bool high) {
    unsigned long temp;
    if(WGMisPWM())
        temp = compare_dbl[idx];
    else
        temp = compare[idx];
    if(high) 
        return (temp >> 8) & 0xff;
    else
        return temp & 0xff;
}

void HWTimer16::SetComplexRegister(bool is_icr, bool high, unsigned char val) {
    if(high)
        accessTempRegister = val;
    else {
        if(is_icr) {
            icapRegister = (accessTempRegister << 8) + val;
            if(wgm == WGM_FASTPWM_ICR)
                limit_top = icapRegister;
        } else
            SetCounter((accessTempRegister << 8) + val);
    }
}

unsigned char HWTimer16::GetComplexRegister(bool is_icr, bool high) {
    if(high)
        return accessTempRegister;
    else {
        if(is_icr) {
            accessTempRegister =  (icapRegister >> 8) & 0xff;
            return icapRegister & 0xff;
        } else {
            accessTempRegister =  (vtcnt >> 8) & 0xff;
            return vtcnt & 0xff;
        }
    }
}

void HWTimer16::ChangeWGM(WGMtype mode) {
    wgm = mode;
    switch(wgm) {
        case WGM_NORMAL:
            limit_top = limit_max;
            break;
            
        case WGM_CTC_OCRA:
            limit_top = compare[0];
            break;
            
        case WGM_CTC_ICR:
            limit_top = icapRegister;
            break;
            
        case WGM_FASTPWM_8BIT:
            limit_top = 0xff;
            break;
            
        case WGM_FASTPWM_9BIT:
            limit_top = 0x1ff;
            break;
            
        case WGM_FASTPWM_10BIT:
            limit_top = 0x3ff;
            break;
            
        case WGM_FASTPWM_OCRA:
            limit_top = compare[0];
            break;
            
        case WGM_FASTPWM_ICR:
            limit_top = icapRegister;
            break;
    }
}

/*
HWTimer8Bit::HWTimer8Bit(AvrDevice *core,
                         PrescalerMultiplexer *p,
                         int unit,
                         IRQLine* tovr):
    Hardware(core),
    core(core),
    premx(p),
    timerOverflow(tovr),
    tcnt_reg(core, "TIMER" + int2str(unit) + ".TCNT",
             this, &HWTimer8Bit::GetTcnt, &HWTimer8Bit::SetTcnt)
{
    Reset();
}

void HWTimer8Bit::CountTimer() {
    last_tcnt = tcnt; // save cycle - 1 counter value
    if(updown_counting) {
        // phase correct mode, counter counts up and down
        if(count_down) {
            tcnt--;
            tcnt_reg.hardwareChange(tcnt);
            if(tcnt == limit_bottom) {
                count_down = false; // now count up
                timerOverflow->fireInterrupt(); // overflow on bottom value,
                                                // set interrupt bit
            }
        } else {
            tcnt++;
            tcnt_reg.hardwareChange(tcnt);
            if(tcnt == limit_top) {
                count_down = true; // now count down
            }
        }
        return;
    }
    // simple up counting till 8Bit overflow
    tcnt++;
    tcnt_reg.hardwareChange(tcnt);
    if(tcnt > 0xff) { // overflow?
        timerOverflow->fireInterrupt();
        tcnt &= 0xff; // reset overflow
    }
}

void HWTimer8Bit::SetTcnt(unsigned char val) {
    tcnt = val;
}

unsigned char HWTimer8Bit::GetTcnt() {
    return tcnt & 0xff;
}

void HWTimer8Bit::Reset() {
    tcnt = 0;
    limit_bottom = 0;
    limit_top = 0xff;
    last_tcnt = limit_top;
    cs = 0;
    updown_counting = false;
    count_down = false;
}

unsigned int HWTimer8Bit::CpuCycle() {
    if(premx->isClock(cs)) {
        CountTimer();
        HandleMode();
    }
    return 0;
}

HWTimer8Bit1OC::HWTimer8Bit1OC(AvrDevice *core,
                               PrescalerMultiplexer *p,
                               int unit,
                               IRQLine* tovr,
                               IRQLine* tcomp,
                               PinAtPort pout):
    HWTimer8Bit(core, p, unit, tovr),
    timerCompare(tcomp),
    ocr_out(pout),
    tccr_reg(core, "TIMER" + int2str(unit) + ".TCCR",
             this, &HWTimer8Bit1OC::GetTccr, &HWTimer8Bit1OC::SetTccr),
    ocr_reg(core, "TIMER" + int2str(unit) + ".OCR",
            this, &HWTimer8Bit1OC::GetOcr, &HWTimer8Bit1OC::SetOcr)
{
    Reset();
}

void HWTimer8Bit1OC::HandleMode(void) {
    // check compare match
    if(last_tcnt == ocr_db) {
        // compare match found
        timerCompare->fireInterrupt(); // OC interrupt will be raised in every case
        // handle compare match output pin
        HandleCompareOutput(com, ocr_out_state, ocr_out);
        // handle counter reset to bottom in CTC mode
        if(wgm == WGM_CTC) {
            tcnt = limit_bottom;
            tcnt_reg.hardwareChange(tcnt);
        }
    }
    
    // handle ocr double buffering
    if(last_tcnt == limit_top && (wgm == WGM_FASTPWM || wgm == WGM_PCPWM))
        ocr_db = ocr;
    
}

void HWTimer8Bit1OC::HandleCompareOutput(COMtype ctrl, bool &out_state, PinAtPort &outpin) {
    if(ctrl == COM_NOOP) return; // nothing to do
    switch(ctrl) {
        case COM_CLEAR:
            out_state = false;
            break;
        case COM_SET:
            out_state = true;
            break;
        case COM_TOGGLE:
            out_state = !out_state;
            break;
    }
    outpin.SetAlternatePort(out_state);
}

void HWTimer8Bit1OC::SetTccr(unsigned char val) {
    tccr = val & 0x7f; // FOCx is allways read as 0!
    
    // handle CSx bits
    cs = val & 0x7; // set select value for prescaler multiplexer
    if(cs != 0) {
        core->AddToCycleList(this);
    } else {
        core->RemoveFromCycleList(this);
    }
    
    // handle WGMx bits
    wgm = (WGMtype)(((val & 0x8) >> 2) + ((val & 0x40) >> 6));
    if(wgm == WGM_PCPWM) // set counter direction mode
      updown_counting = true;
    else
      updown_counting = false;
    
    // handle COMx bits
    com = (COMtype)((val & 0x30) >> 4);
    if((com == COM_TOGGLE) && (wgm == WGM_PCPWM || wgm == WGM_FASTPWM)) {
        avr_warning("toggle mode in pwm operation mode is reserved, set it to NOOP");
        com = COM_NOOP;
    }
    if(com == COM_NOOP)
        ocr_out.SetUseAlternatePortIfDdrSet(false); // normal pin mode
    else {
        ocr_out.SetUseAlternatePortIfDdrSet(true); // alternate mode
        ocr_out.SetAlternatePort(ocr_out_state);
    }
    
    // handle FOCx bit
    if(val & 0x80)
        HandleCompareOutput(com, ocr_out_state, ocr_out);
}

unsigned char HWTimer8Bit1OC::GetTccr() {
    return tccr;
}

void HWTimer8Bit1OC::SetOcr(unsigned char val) {
    ocr = val;
    if(wgm == WGM_NORMAL || wgm == WGM_CTC) ocr_db = val; // immediate update
}

unsigned char HWTimer8Bit1OC::GetOcr() {
    return ocr;
}

void HWTimer8Bit1OC::Reset() {
    HWTimer8Bit::Reset();
    ocr = 0;
    ocr_db = 0;
    ocr_out_state = false;
    tccr = 0;
    wgm = WGM_NORMAL;
    com = COM_NOOP;
}
*/

HWTimer8_1C::HWTimer8_1C(AvrDevice *core,
                         PrescalerMultiplexer *p,
                         int unit,
                         IRQLine* tov,
                         IRQLine* tcompA,
                         PinAtPort* outA):
    HWTimer8(core, p, unit, tov, tcompA, outA, NULL, NULL),
    tccr_reg(core, GetTraceValuePrefix() + "TCCR",
             this, &HWTimer8_1C::Get_TCCR, &HWTimer8_1C::Set_TCCR) {}

void HWTimer8_1C::Set_TCCR(unsigned char val) {
    WGMtype temp;
    int raw_wgm = ((val & 0x8) >> 2) + ((val & 0x40) >> 6);
    switch(raw_wgm) {
        case 0: temp = WGM_NORMAL;       break;
        case 1: temp = WGM_PCPWM_8BIT;   break;
        case 2: temp = WGM_CTC_OCRA;     break;
        case 3: temp = WGM_FASTPWM_8BIT; break;
    }
    if(wgm != temp)
        ChangeWGM((WGMtype)temp);
    SetCompareOutputMode(0, (COMtype)((val >> 4) & 0x3));
    SetClockMode(val & 0x7);
    if(!WGMisPWM() && val & 0x80)
      // FOCx
      SetCompareOutput(0);
    
    tccr_val = val & 0x7f;
}

void HWTimer8_1C::Reset() {
    HWTimer8::Reset();
    tccr_val = 0;
}

HWTimer16_3C::HWTimer16_3C(AvrDevice *core,
                           PrescalerMultiplexer *p,
                           int unit,
                           IRQLine* tov,
                           IRQLine* tcompA,
                           PinAtPort* outA,
                           IRQLine* tcompB,
                           PinAtPort* outB,
                           IRQLine* tcompC,
                           PinAtPort* outC,
                           IRQLine* ticap):
    HWTimer16(core, p, unit, tov, tcompA, outA, tcompB, outB, tcompC, outC, ticap),
    tccra_reg(core, GetTraceValuePrefix() + "TCCRA",
              this, &HWTimer16_3C::Get_TCCRA, &HWTimer16_3C::Set_TCCRA),
    tccrb_reg(core, GetTraceValuePrefix() + "TCCRB",
              this, &HWTimer16_3C::Get_TCCRB, &HWTimer16_3C::Set_TCCRB),
    tccrc_reg(core, GetTraceValuePrefix() + "TCCRC",
              this, &HWTimer16_3C::Get_TCCRC, &HWTimer16_3C::Set_TCCRC) {}

void HWTimer16_3C::Set_TCCRA(unsigned char val) {
    int temp = (int)wgm;
    temp &= ~0x3;
    temp += val & 0x3;
    if(wgm != (WGMtype)temp)
        ChangeWGM((WGMtype)temp);
    SetCompareOutputMode(0, (COMtype)((val >> 6) & 0x3));
    SetCompareOutputMode(1, (COMtype)((val >> 4) & 0x3));
    SetCompareOutputMode(2, (COMtype)((val >> 2) & 0x3));
    
    tccra_val = val;
}

void HWTimer16_3C::Set_TCCRB(unsigned char val) {
    int temp = (int)wgm;
    temp &= ~0xc;
    temp += (val >> 1) & 0xc;
    if(wgm != (WGMtype)temp)
        ChangeWGM((WGMtype)temp);
    SetClockMode(val & 0x7);

    tccrb_val = val;
}

void HWTimer16_3C::Set_TCCRC(unsigned char val) {
    if(!WGMisPWM()) {
        if(val & 0x80)
            // FOCxA
            SetCompareOutput(0);
        if(val & 0x40)
            // FOCxB
            SetCompareOutput(1);
        if(val & 0x20)
            // FOCxC
            SetCompareOutput(2);
    }
}

void HWTimer16_3C::Reset() {
    HWTimer16::Reset();
    tccra_val = 0;
    tccrb_val = 0;
}


