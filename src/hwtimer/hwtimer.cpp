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

#include "../hardware.h"
#include "../avrdevice.h"
#include "timerprescaler.h"
#include "hwtimer.h"
#include "../helper.h"
#include "systemclock.h"

#include <cstdlib>
#include <time.h>

using namespace std;

BasicTimerUnit::BasicTimerUnit(AvrDevice *core,
                               PrescalerMultiplexer *p,
                               int unit,
                               IRQLine* tov,
                               IRQLine* tcap,
                               ICaptureSource* icapsrc,
                               int countersize):
    Hardware(core),
    TraceValueRegister(core, "TIMER" + int2str(unit)),
    core(core),
    premx(p),
    timerOverflow(tov),
    timerCapture(tcap),
    icapSource(icapsrc)
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
    
    // disable all compare registers, output pins and reset Compare IRQ's
    for(int i = 0; i < OCRIDX_maxUnits; i++) {
        compareEnable[i] = false;
        timerCompare[i] = NULL;
        compare_output[i] = NULL;
    }
    
    // set wgm functions
    for(int i = 0; i < WGM_tablesize; i++)
        wgmfunc[i] = &BasicTimerUnit::WGMFunc_noop;
    
    // set saved input capture state and variables for noise canceler
    captureInputState = false;
    icapNCcounter = 0;
    icapNCstate = false;
    
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

void BasicTimerUnit::InputCapture(void) {
    if(icapSource != NULL && !WGMuseICR()) {
        // get the current state
        bool tmp = icapSource->GetSourceState();
        if(icapNoiseCanceler) {
            // use noise canceler
            if(tmp == icapNCstate) {
                if(icapNCcounter < 4) {
                    icapNCcounter++;
                    tmp = captureInputState; // do not trigger event!
                }
            } else {
                // state change, reset counter
                icapNCcounter = 0;
                icapNCstate = tmp;
                tmp = captureInputState; // do not trigger event!
            }
        }
        
        // detect change
        if(tmp != captureInputState) {
            if(tmp == icapRisingEdge) {
                // right edge seen, capture timer counter
                icapRegister = vtcnt;
                // fire capture interrupt
                if(timerCapture)
                    timerCapture->fireInterrupt();
            }
            captureInputState = tmp;
        }
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
    bool new_state=false, old_state = compare_output_state[idx];
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

void BasicTimerUnit::SetPWMCompareOutput(int idx, bool topOrDown) {
    COMtype mode = com[idx];
    bool new_state=false, old_state = compare_output_state[idx];
    switch(mode) {
        case COM_NOOP:
            return;
            
        case COM_TOGGLE:
            if((wgm == WGM_FASTPWM_OCRA ||
                wgm == WGM_PCPWM_OCRA ||
                wgm == WGM_PFCPWM_OCRA) && idx == OCRIDX_A)
                // special mode in case of WGM_FASTPWM_OCRA!
                SetCompareOutput(OCRIDX_A);
            else
                avr_warning("COM==1 in PWM mode is reserved!");
            break;
            
        case COM_CLEAR:
            if(topOrDown)
                new_state = true;
            else
                new_state = false;
            break;
            
        case COM_SET:
            if(topOrDown)
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
            for(int i = 0; i < OCRIDX_maxUnits; i++)
                SetPWMCompareOutput(i, true);
            // reset counter
            vtcnt = limit_bottom;
            break;
            
        case EVT_BOTTOM_REACHED:
            // update OC registers
            for(int i = 0; i < OCRIDX_maxUnits; i++) {
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
                    else
                        compare[OCRIDX_A] = compare_dbl[OCRIDX_A];
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

void BasicTimerUnit::WGMfunc_pcpwm(CEtype event) {
    switch(event) {
        case EVT_TOP_REACHED:
            // if ICR or OCRA mode, fire this interrupts
            if(wgm == WGM_PCPWM_OCRA) {
                if(timerCompare[OCRIDX_A])
                    timerCompare[OCRIDX_A]->fireInterrupt();
            } else if(wgm == WGM_PCPWM_ICR) {
                if(timerCapture)
                    timerCapture->fireInterrupt();
            }
            // update OC registers
            for(int i = 0; i < OCRIDX_maxUnits; i++) {
                if(i == OCRIDX_A) {
                    if(wgm == WGM_PCPWM_8BIT)
                        // mask to 0xff
                        compare[i] = compare_dbl[i] & 0xff;
                    else if(wgm == WGM_PCPWM_9BIT)
                        // mask to 0x1ff
                        compare[i] = compare_dbl[i] & 0x1ff;
                    else if(wgm == WGM_PCPWM_10BIT)
                        // mask to 0x3ff
                        compare[i] = compare_dbl[i] & 0x3ff;
                    else if(wgm == WGM_PCPWM_OCRA) {
                        // set new top value
                        limit_top = compare_dbl[i];
                        // and process output
                        SetPWMCompareOutput(0, false);
                    } else
                        compare[OCRIDX_A] = compare_dbl[OCRIDX_A];
                      
                } else
                    compare[i] = compare_dbl[i];
            }
            break;
          
        case EVT_BOTTOM_REACHED:
            // fire overflow interrupt
            timerOverflow->fireInterrupt();
            break;
          
        case EVT_COMPARE_1:
            if(timerCompare[0] && wgm != WGM_PCPWM_OCRA) {
                timerCompare[0]->fireInterrupt();
                SetPWMCompareOutput(0, count_down);
            }
            break;
            
        case EVT_COMPARE_2:
            if(timerCompare[1]) {
                timerCompare[1]->fireInterrupt();
                SetPWMCompareOutput(1, count_down);
            }
            break;
            
        case EVT_COMPARE_3:
            if(timerCompare[2]) {
                timerCompare[2]->fireInterrupt();
                SetPWMCompareOutput(2, count_down);
            }
            break;
            
        default:
            break;
    }
}

void BasicTimerUnit::WGMfunc_pfcpwm(CEtype event) {
    switch(event) {
        case EVT_TOP_REACHED:
            // if ICR or OCRA mode, fire this interrupts
            if(wgm == WGM_PFCPWM_OCRA) {
                if(timerCompare[OCRIDX_A])
                    timerCompare[OCRIDX_A]->fireInterrupt();
            } else if(wgm == WGM_PFCPWM_ICR) {
                if(timerCapture)
                    timerCapture->fireInterrupt();
            }
            // process output from OC A in case of WGM_PFCPWM_OCRA
            if(wgm == WGM_PFCPWM_OCRA)
                SetPWMCompareOutput(0, false);
            break;
          
        case EVT_BOTTOM_REACHED:
            // fire overflow interrupt
            timerOverflow->fireInterrupt();
            // update OC registers
            for(int i = 0; i < OCRIDX_maxUnits; i++) {
                if(i == OCRIDX_A) {
                    if(wgm == WGM_PFCPWM_OCRA)
                        // set new top value
                        limit_top = compare_dbl[i];
                    else
                        compare[OCRIDX_A] = compare_dbl[OCRIDX_A];
                } else
                    compare[i] = compare_dbl[i];
            }
            break;
          
        case EVT_COMPARE_1:
            if(timerCompare[0] && wgm != WGM_PFCPWM_OCRA) {
                timerCompare[0]->fireInterrupt();
                SetPWMCompareOutput(0, count_down);
            }
            break;
            
        case EVT_COMPARE_2:
            if(timerCompare[1]) {
                timerCompare[1]->fireInterrupt();
                SetPWMCompareOutput(1, count_down);
            }
            break;
            
        case EVT_COMPARE_3:
            if(timerCompare[2]) {
                timerCompare[2]->fireInterrupt();
                SetPWMCompareOutput(2, count_down);
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
    icapRisingEdge = false;
    icapNoiseCanceler = false;
}

unsigned int BasicTimerUnit::CpuCycle() {
    if(premx->isClock(cs))
        CountTimer();
    InputCapture();
    return 0;
}

void BasicTimerUnit::RegisterACompForICapture(HWAcomp *acomp) {
    if(icapSource != NULL)
        icapSource->RegisterAComp(acomp);
}

HWTimer8::HWTimer8(AvrDevice *core,
                   PrescalerMultiplexer *p,
                   int unit,
                   IRQLine* tov,
                   IRQLine* tcompA,
                   PinAtPort* outA,
                   IRQLine* tcompB,
                   PinAtPort* outB):
    BasicTimerUnit(core, p, unit, tov, NULL, NULL, 8),
    tcnt_reg(this, "TCNT",
             this, &HWTimer8::Get_TCNT, &HWTimer8::Set_TCNT),
    ocra_reg(this, "OCRA",
             this, &HWTimer8::Get_OCRA, &HWTimer8::Set_OCRA),
    ocrb_reg(this, "OCRB",
             this, &HWTimer8::Get_OCRB, &HWTimer8::Set_OCRB)
{
    // enable OC units and disable registers
    if(tcompA) {
        compareEnable[0] = true;
        timerCompare[0] = tcompA;
        compare_output[0] = outA;
    } else
      ocra_reg.releaseTraceValue();
    if(tcompB) {
        compareEnable[1] = true;
        timerCompare[1] = tcompB;
        compare_output[1] = outB;
    } else
      ocrb_reg.releaseTraceValue();
    
    // set WGM table
    wgmfunc[WGM_NORMAL] = &HWTimer8::WGMfunc_normal;
    wgmfunc[WGM_CTC_OCRA] = &HWTimer8::WGMfunc_ctc;
    wgmfunc[WGM_FASTPWM_8BIT] = &HWTimer8::WGMfunc_fastpwm;
    wgmfunc[WGM_PCPWM_8BIT] = &HWTimer8::WGMfunc_pcpwm;
    
    // reset unit
    Reset();
}

void HWTimer8::Reset(void) {
    BasicTimerUnit::Reset();
}

void HWTimer8::ChangeWGM(WGMtype mode) {
    wgm = mode;
    switch(wgm) {
        case WGM_PCPWM_9BIT:
        case WGM_PCPWM_10BIT:
        case WGM_FASTPWM_9BIT:
        case WGM_FASTPWM_10BIT:
        case WGM_PFCPWM_ICR:
        case WGM_PFCPWM_OCRA:
        case WGM_PCPWM_ICR:
        case WGM_PCPWM_OCRA:
        case WGM_CTC_ICR:
        case WGM_RESERVED:
        case WGM_FASTPWM_ICR:
        case WGM_FASTPWM_OCRA:
        case WGM_tablesize: 
            break;

        case WGM_NORMAL:
            limit_top = limit_max;
            updown_counting = false;
            break;
            
        case WGM_CTC_OCRA:
            limit_top = compare[0];
            updown_counting = false;
            break;
            
        case WGM_FASTPWM_8BIT:
            limit_top = limit_max;
            updown_counting = false;
            break;
            
        case WGM_PCPWM_8BIT:
            limit_top = limit_max;
            updown_counting = true;
            count_down = false;
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
                     IRQLine* ticap,
                     ICaptureSource* icapsrc):
    BasicTimerUnit(core, p, unit, tov, ticap, icapsrc, 16),
    tcnt_h_reg(this, "TCNTH",
               this, &HWTimer16::Get_TCNTH, &HWTimer16::Set_TCNTH),
    tcnt_l_reg(this, "TCNTL",
               this, &HWTimer16::Get_TCNTL, &HWTimer16::Set_TCNTL),
    ocra_h_reg(this, "OCRAH",
               this, &HWTimer16::Get_OCRAH, &HWTimer16::Set_OCRAH),
    ocra_l_reg(this, "OCRAL",
               this, &HWTimer16::Get_OCRAL, &HWTimer16::Set_OCRAL),
    ocrb_h_reg(this, "OCRBH",
               this, &HWTimer16::Get_OCRBH, &HWTimer16::Set_OCRBH),
    ocrb_l_reg(this, "OCRBL",
               this, &HWTimer16::Get_OCRBL, &HWTimer16::Set_OCRBL),
    ocrc_h_reg(this, "OCRCH",
               this, &HWTimer16::Get_OCRCH, &HWTimer16::Set_OCRCH),
    ocrc_l_reg(this, "OCRCL",
               this, &HWTimer16::Get_OCRCL, &HWTimer16::Set_OCRCL),
    icr_h_reg(this, "ICRH",
              this, &HWTimer16::Get_ICRH, &HWTimer16::Set_ICRH),
    icr_l_reg(this, "ICRL",
              this, &HWTimer16::Get_ICRL, &HWTimer16::Set_ICRL)
{
    // enable OC units and disable registers
    if(tcompA) {
        compareEnable[0] = true;
        timerCompare[0] = tcompA;
        compare_output[0] = outA;
    } else {
      ocra_l_reg.releaseTraceValue();
      ocra_h_reg.releaseTraceValue();
    }
    if(tcompB) {
        compareEnable[1] = true;
        timerCompare[1] = tcompB;
        compare_output[1] = outB;
    } else {
      ocrb_l_reg.releaseTraceValue();
      ocrb_h_reg.releaseTraceValue();
    }
    if(tcompC) {
        compareEnable[2] = true;
        timerCompare[2] = tcompC;
        compare_output[2] = outC;
    } else {
      ocrc_l_reg.releaseTraceValue();
      ocrc_h_reg.releaseTraceValue();
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
    wgmfunc[WGM_PCPWM_8BIT] = &HWTimer16::WGMfunc_pcpwm;
    wgmfunc[WGM_PCPWM_9BIT] = &HWTimer16::WGMfunc_pcpwm;
    wgmfunc[WGM_PCPWM_10BIT] = &HWTimer16::WGMfunc_pcpwm;
    wgmfunc[WGM_PCPWM_OCRA] = &HWTimer16::WGMfunc_pcpwm;
    wgmfunc[WGM_PCPWM_ICR] = &HWTimer16::WGMfunc_pcpwm;
    wgmfunc[WGM_PFCPWM_OCRA] = &HWTimer16::WGMfunc_pfcpwm;
    wgmfunc[WGM_PFCPWM_ICR] = &HWTimer16::WGMfunc_pfcpwm;
    
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
    if(high) {
        if(is_icr && !WGMuseICR())
            avr_warning("ICRxH isn't writable in a non-ICR WGM mode");
        else
            accessTempRegister = val;
    } else {
        if(is_icr) {
            if(WGMuseICR()) {
                icapRegister = (accessTempRegister << 8) + val;
                if(wgm == WGM_FASTPWM_ICR)
                    limit_top = icapRegister;
            } else
                avr_warning("ICRxL isn't writable in a non-ICR WGM mode");
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
        case WGM_RESERVED:
        case WGM_tablesize:
            break;

        case WGM_NORMAL:
            limit_top = limit_max;
            updown_counting = false;
            break;
            
        case WGM_CTC_OCRA:
            limit_top = compare[0];
            updown_counting = false;
            break;
            
        case WGM_CTC_ICR:
            limit_top = icapRegister;
            updown_counting = false;
            break;
            
        case WGM_FASTPWM_8BIT:
            limit_top = 0xff;
            updown_counting = false;
            break;
            
        case WGM_FASTPWM_9BIT:
            limit_top = 0x1ff;
            updown_counting = false;
            break;
            
        case WGM_FASTPWM_10BIT:
            limit_top = 0x3ff;
            updown_counting = false;
            break;
            
        case WGM_FASTPWM_OCRA:
            limit_top = compare[0];
            updown_counting = false;
            break;
            
        case WGM_FASTPWM_ICR:
            limit_top = icapRegister;
            updown_counting = false;
            break;
            
        case WGM_PCPWM_8BIT:
            limit_top = 0xff;
            updown_counting = true;
            count_down = false;
            break;
            
        case WGM_PCPWM_9BIT:
            limit_top = 0x1ff;
            updown_counting = true;
            count_down = false;
            break;
            
        case WGM_PCPWM_10BIT:
            limit_top = 0x3ff;
            updown_counting = true;
            count_down = false;
            break;
            
        case WGM_PCPWM_OCRA:
        case WGM_PFCPWM_OCRA:
            limit_top = compare[0];
            updown_counting = true;
            count_down = false;
            break;
            
        case WGM_PCPWM_ICR:
        case WGM_PFCPWM_ICR:
            limit_top = icapRegister;
            updown_counting = true;
            count_down = false;
            break;
    }
}

HWTimer8_0C::HWTimer8_0C(AvrDevice *core,
                         PrescalerMultiplexer *p,
                         int unit,
                         IRQLine* tov):
    HWTimer8(core, p, unit, tov, NULL, NULL, NULL, NULL),
    tccr_reg(this, "TCCR",
             this, &HWTimer8_0C::Get_TCCR, &HWTimer8_0C::Set_TCCR)
{
    ChangeWGM(WGM_NORMAL);
}

void HWTimer8_0C::Set_TCCR(unsigned char val) {
    SetClockMode(val & 0x7);
    tccr_val = val;
}

void HWTimer8_0C::Reset() {
    HWTimer8::Reset();
    tccr_val = 0;
}

HWTimer8_1C::HWTimer8_1C(AvrDevice *core,
                         PrescalerMultiplexer *p,
                         int unit,
                         IRQLine* tov,
                         IRQLine* tcompA,
                         PinAtPort* outA):
    HWTimer8(core, p, unit, tov, tcompA, outA, NULL, NULL),
    tccr_reg(this, "TCCR",
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

HWTimer8_2C::HWTimer8_2C(AvrDevice *core,
                         PrescalerMultiplexer *p,
                         int unit,
                         IRQLine* tov,
                         IRQLine* tcompA,
                         PinAtPort* outA,
                         IRQLine* tcompB,
                         PinAtPort* outB):
    HWTimer8(core, p, unit, tov, tcompA, outA, tcompB, outB),
    tccra_reg(this, "TCCRA",
             this, &HWTimer8_2C::Get_TCCRA, &HWTimer8_2C::Set_TCCRA),
    tccrb_reg(this, "TCCRB",
             this, &HWTimer8_2C::Get_TCCRB, &HWTimer8_2C::Set_TCCRB) {}

void HWTimer8_2C::Set_WGM(int val) {
    WGMtype w;
    if(wgm_raw != val) {
        // translate WGM modes
        switch(val & 0x7) {
            case 0: w = WGM_NORMAL;       break;
            case 1: w = WGM_PCPWM_8BIT;   break;
            case 2: w = WGM_CTC_OCRA;     break;
            case 3: w = WGM_FASTPWM_8BIT; break;
            case 4: w = WGM_RESERVED;     break;
            case 5: w = WGM_PCPWM_OCRA;   break;
            case 6: w = WGM_RESERVED;     break;
            case 7: w = WGM_FASTPWM_OCRA; break;
        }
        ChangeWGM(w);
        // save new raw value
        wgm_raw = val;
    }
}

void HWTimer8_2C::Set_TCCRA(unsigned char val) {
    int temp = wgm_raw;
    temp &= ~0x3;
    temp += val & 0x3;
    Set_WGM(temp);
    SetCompareOutputMode(0, (COMtype)((val >> 6) & 0x3));
    SetCompareOutputMode(1, (COMtype)((val >> 4) & 0x3));
    
    tccra_val = val;
}

void HWTimer8_2C::Set_TCCRB(unsigned char val) {
    int temp = wgm_raw;
    temp &= ~0x4;
    temp += (val >> 1) & 0x4;
    Set_WGM(temp);
    SetClockMode(val & 0x7);
    if(!WGMisPWM()) {
        if(val & 0x80)
            // FOCxA
            SetCompareOutput(0);
        if(val & 0x40)
            // FOCxB
            SetCompareOutput(1);
    }

    tccrb_val = val & 0x3f;
}

void HWTimer8_2C::Reset() {
    HWTimer8::Reset();
    tccra_val = 0;
    tccrb_val = 0;
    wgm_raw = 0;
}

HWTimer16_1C::HWTimer16_1C(AvrDevice *core,
                           PrescalerMultiplexer *p,
                           int unit,
                           IRQLine* tov,
                           IRQLine* tcompA,
                           PinAtPort* outA,
                           IRQLine* ticap,
                           ICaptureSource* icapsrc):
    HWTimer16(core, p, unit, tov, tcompA, outA, NULL, NULL, NULL, NULL, ticap, icapsrc),
    tccra_reg(this, "TCCRA",
              this, &HWTimer16_1C::Get_TCCRA, &HWTimer16_1C::Set_TCCRA),
    tccrb_reg(this, "TCCRB",
              this, &HWTimer16_1C::Get_TCCRB, &HWTimer16_1C::Set_TCCRB) {}

void HWTimer16_1C::Set_WGM(int val) {
    WGMtype w;
    if(wgm_raw != val) {
        // translate WGM modes
        switch(val & 0x7) {
            case 0: w = WGM_NORMAL;      break;
            case 1: w = WGM_PCPWM_8BIT;  break;
            case 2: w = WGM_PCPWM_9BIT;  break;
            case 3: w = WGM_PCPWM_10BIT; break;
            case 4: w = WGM_CTC_OCRA;    break;
            case 5: w = WGM_PCPWM_8BIT;  break;
            case 6: w = WGM_PCPWM_9BIT;  break;
            case 7: w = WGM_PCPWM_10BIT; break;
        }
        ChangeWGM(w);
        // save new raw value
        wgm_raw = val;
    }
}

void HWTimer16_1C::Set_TCCRA(unsigned char val) {
    int temp = wgm_raw;
    temp &= ~0x3;
    temp += val & 0x3;
    Set_WGM(temp);
    SetCompareOutputMode(0, (COMtype)((val >> 6) & 0x3));
    
    tccra_val = val;
}

void HWTimer16_1C::Set_TCCRB(unsigned char val) {
    int temp = wgm_raw;
    temp &= ~0x4;
    temp += (val >> 1) & 0x4;
    Set_WGM(temp);
    SetClockMode(val & 0x7);
    icapNoiseCanceler = (val & 0x80) == 0x80;
    icapRisingEdge = (val & 0x40) == 0x40;
    
    tccrb_val = val;
}

void HWTimer16_1C::Reset() {
    HWTimer16::Reset();
    tccra_val = 0;
    tccrb_val = 0;
    wgm_raw = 0;
}

HWTimer16_2C2::HWTimer16_2C2(AvrDevice *core,
                             PrescalerMultiplexer *p,
                             int unit,
                             IRQLine* tov,
                             IRQLine* tcompA,
                             PinAtPort* outA,
                             IRQLine* tcompB,
                             PinAtPort* outB,
                             IRQLine* ticap,
                             ICaptureSource* icapsrc,
                             bool is_at8515):
    HWTimer16(core, p, unit, tov, tcompA, outA, tcompB, outB, NULL, NULL, ticap, icapsrc),
    at8515_mode(is_at8515),
    tccra_reg(this, "TCCRA",
              this, &HWTimer16_2C2::Get_TCCRA, &HWTimer16_2C2::Set_TCCRA),
    tccrb_reg(this, "TCCRB",
              this, &HWTimer16_2C2::Get_TCCRB, &HWTimer16_2C2::Set_TCCRB)
{}

void HWTimer16_2C2::Set_WGM(int val) {
    if(wgm_raw != val) {
        // translate WGM modes
        if(at8515_mode) {
            WGMtype w;
            switch(val & 0x7) {
                case 0: w = WGM_NORMAL;      break;
                case 1: w = WGM_PCPWM_8BIT;  break;
                case 2: w = WGM_PCPWM_9BIT;  break;
                case 3: w = WGM_PCPWM_10BIT; break;
                case 4: w = WGM_CTC_OCRA;    break;
                case 5: w = WGM_PCPWM_8BIT;  break;
                case 6: w = WGM_PCPWM_9BIT;  break;
                case 7: w = WGM_PCPWM_10BIT; break;
            }
            ChangeWGM(w);
        } else
            ChangeWGM((WGMtype)val);
        // save new raw value
        wgm_raw = val;
    }
}

void HWTimer16_2C2::Set_TCCRA(unsigned char val) {
    int temp = wgm_raw;
    temp &= ~0x3;
    temp += val & 0x3;
    Set_WGM(temp);
    SetCompareOutputMode(0, (COMtype)((val >> 6) & 0x3));
    SetCompareOutputMode(1, (COMtype)((val >> 4) & 0x3));
    if(!WGMisPWM() && !at8515_mode) {
        if(val & 0x08)
            // FOCxA
            SetCompareOutput(0);
        if(val & 0x04)
            // FOCxB
            SetCompareOutput(1);
    }
    
    tccra_val = val;
}

void HWTimer16_2C2::Set_TCCRB(unsigned char val) {
    int mask = at8515_mode ? 0x4 : 0xc;
    int temp = wgm_raw;
    temp &= ~mask;
    temp += (val >> 1) & mask;
    Set_WGM(temp);
    SetClockMode(val & 0x7);
    icapNoiseCanceler = (val & 0x80) == 0x80;
    icapRisingEdge = (val & 0x40) == 0x40;

    tccrb_val = val;
}

void HWTimer16_2C2::Reset() {
    HWTimer16::Reset();
    tccra_val = 0;
    tccrb_val = 0;
    wgm_raw = 0;
}

HWTimer16_2C3::HWTimer16_2C3(AvrDevice *core,
                             PrescalerMultiplexer *p,
                             int unit,
                             IRQLine* tov,
                             IRQLine* tcompA,
                             PinAtPort* outA,
                             IRQLine* tcompB,
                             PinAtPort* outB,
                             IRQLine* ticap,
                             ICaptureSource* icapsrc):
    HWTimer16(core, p, unit, tov, tcompA, outA, tcompB, outB, NULL, NULL, ticap, icapsrc),
    tccra_reg(this, "TCCRA",
              this, &HWTimer16_2C3::Get_TCCRA, &HWTimer16_2C3::Set_TCCRA),
    tccrb_reg(this, "TCCRB",
              this, &HWTimer16_2C3::Get_TCCRB, &HWTimer16_2C3::Set_TCCRB),
    tccrc_reg(this, "TCCRC",
              this, &HWTimer16_2C3::Get_TCCRC, &HWTimer16_2C3::Set_TCCRC) {}

void HWTimer16_2C3::Set_TCCRA(unsigned char val) {
    int temp = (int)wgm;
    temp &= ~0x3;
    temp += val & 0x3;
    if(wgm != (WGMtype)temp)
        ChangeWGM((WGMtype)temp);
    SetCompareOutputMode(0, (COMtype)((val >> 6) & 0x3));
    SetCompareOutputMode(1, (COMtype)((val >> 4) & 0x3));
    
    tccra_val = val;
}

void HWTimer16_2C3::Set_TCCRB(unsigned char val) {
    int temp = (int)wgm;
    temp &= ~0xc;
    temp += (val >> 1) & 0xc;
    if(wgm != (WGMtype)temp)
        ChangeWGM((WGMtype)temp);
    SetClockMode(val & 0x7);
    icapNoiseCanceler = (val & 0x80) == 0x80;
    icapRisingEdge = (val & 0x40) == 0x40;

    tccrb_val = val;
}

void HWTimer16_2C3::Set_TCCRC(unsigned char val) {
    if(!WGMisPWM()) {
        if(val & 0x80)
            // FOCxA
            SetCompareOutput(0);
        if(val & 0x40)
            // FOCxB
            SetCompareOutput(1);
    }
}

void HWTimer16_2C3::Reset() {
    HWTimer16::Reset();
    tccra_val = 0;
    tccrb_val = 0;
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
                           IRQLine* ticap,
                           ICaptureSource* icapsrc):
    HWTimer16(core, p, unit, tov, tcompA, outA, tcompB, outB, tcompC, outC, ticap, icapsrc),
    tccra_reg(this, "TCCRA",
              this, &HWTimer16_3C::Get_TCCRA, &HWTimer16_3C::Set_TCCRA),
    tccrb_reg(this, "TCCRB",
              this, &HWTimer16_3C::Get_TCCRB, &HWTimer16_3C::Set_TCCRB),
    tccrc_reg(this, "TCCRC",
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
    icapNoiseCanceler = (val & 0x80) == 0x80;
    icapRisingEdge = (val & 0x40) == 0x40;

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

//! Step time in ns for async clock by pll
/*! Because system clock steps are counted in ns, we have to calculate so many steps to get
 * over all steps a time in ns without fraction. For 64MHz, e.g. 15,625 ns period, this step
 * time without fraction is 125ns, e.g. 8 different steps. For LSM mode and 32MHz we take
 * every 2 values together for one step.
 */
const int HWTimerTinyX5_nextdelay[8] = { 16, 15, 16, 16, 15, 16, 15, 16 };

HWTimerTinyX5::HWTimerTinyX5(AvrDevice *core,
        IOSpecialReg *gtccr,
        IOSpecialReg *pllcsr,
        IRQLine* tov,
        IRQLine* tocra,
        PinAtPort* ocra_out,
        PinAtPort* ocra_outinv,
        IRQLine* tocrb,
        PinAtPort* ocrb_out,
        PinAtPort* ocrb_outinv):
    Hardware(core),
    TraceValueRegister(core, "TIMER1"),
    ocra_unit(ocra_out, ocra_outinv),
    ocrb_unit(ocrb_out, ocrb_outinv),
    core(core),
    timerOverflowInt(tov),
    timerOCRAInt(tocra),
    timerOCRBInt(tocrb),
    tccr_reg(this, "TCCR1",
             this, &HWTimerTinyX5::Get_TCCR, &HWTimerTinyX5::Set_TCCR),
    tcnt_reg(this, "TCNT1",
             this, &HWTimerTinyX5::Get_TCNT, &HWTimerTinyX5::Set_TCNT),
    tocra_reg(this, "OCR1A",
              this, &HWTimerTinyX5::Get_OCRA, &HWTimerTinyX5::Set_OCRA),
    tocrb_reg(this, "OCR1B",
              this, &HWTimerTinyX5::Get_OCRB, &HWTimerTinyX5::Set_OCRB),
    tocrc_reg(this, "OCR1C",
              this, &HWTimerTinyX5::Get_OCRC, &HWTimerTinyX5::Set_OCRC),
    dtps1_reg(this, "DTPS1",
              this, &HWTimerTinyX5::Get_DTPS1, &HWTimerTinyX5::Set_DTPS1),
    dt1a_reg(this, "DT1A",
             this, &HWTimerTinyX5::Get_DT1A, &HWTimerTinyX5::Set_DT1A),
    dt1b_reg(this, "DT1B",
             this, &HWTimerTinyX5::Get_DT1B, &HWTimerTinyX5::Set_DT1B)
{
    // gtccr and pllcsr register
    gtccrRegister = gtccr;
    gtccrRegister->connectSRegClient(this);
    pllcsrRegister = pllcsr;
    pllcsrRegister->connectSRegClient(this);

    // create TraceValue for counter itself and for prescaler
    counterTrace = new TraceValue(8, GetTraceValuePrefix() + "Counter");
    RegisterTraceValue(counterTrace);
    counterTrace->set_written(0);
    prescalerTrace = new TraceValue(14, GetTraceValuePrefix() + "Prescaler");
    RegisterTraceValue(prescalerTrace);
    prescalerTrace->set_written(0);
    dTPrescalerTrace = new TraceValue(3, GetTraceValuePrefix() + "DeadTimePrescaler");
    RegisterTraceValue(dTPrescalerTrace);
    dTPrescalerTrace->set_written(0);

    // connect to core to get core cycles
    core->AddToCycleList(this);

    // pre initialization for async mode, starts with sync mode
    asyncClock_async = false;
    asyncClock_step = -1;

    // reset internal values
    Reset();
}

HWTimerTinyX5::~HWTimerTinyX5() {
    delete dTPrescalerTrace;
    delete prescalerTrace;
    delete counterTrace;
}

void HWTimerTinyX5::Reset() {
    counter = 0;
    tcnt_out_val = tcnt_in_val = 0;
    tov_internal_flag = tocra_internal_flag = tocrb_internal_flag = false;
    prescaler = 0;
    dtprescaler = 0;
    tccr_inout_val.Reset(0);
    cfg_prescaler = 0;
    cfg_mode = TMODE_NORMAL;
    cfg_com_a = cfg_com_b = 0;
    cfg_ctc = false;
    ocra_inout_val.Reset(0);
    ocra_internal_val = 0;
    ocrb_inout_val.Reset(0);
    ocrb_internal_val = 0;
    ocrc_inout_val.Reset(0xff);
    dtps1_inout_val = 0;
    dt1a_inout_val.Reset(0);
    dt1b_inout_val.Reset(0);
    gtccr_in_val.Reset(0);
    asyncClock_pll = asyncClock_plllock = false;

    ocra_unit.Reset();
    ocrb_unit.Reset();

    SetPrescalerClock(false); // reset prescaler to sync. clock mode, if necessary!
}

int HWTimerTinyX5::Step(bool &untilCoreStepFinished, SystemClockOffset *nextStepIn_ns) {
    if(asyncClock_async) {
        *nextStepIn_ns = HWTimerTinyX5_nextdelay[asyncClock_step];
        asyncClock_step++;
        if(asyncClock_lsm) {
            *nextStepIn_ns += HWTimerTinyX5_nextdelay[asyncClock_step];
            asyncClock_step++;
        }
        if(asyncClock_step == 8) asyncClock_step = 0;
        // process prescaler and timer
        TimerCounter();
        // dump_manager->cycle() needed to get correct trace display
        DumpManager::Instance()->cycle();
        // transfer input register values to real used operation registers (pll clock delay 1 step in async mode)
        TransferInputValues();
    } else {
        // switch to sync clock
        asyncClock_step = -1; // this activates timer calculation in CpuCycle()
        *nextStepIn_ns = -1; // this removes timer from syncMembers list
    }
    return 0;
}

unsigned int HWTimerTinyX5::CpuCycle() {
    // transfer output register values to readable register by core
    TransferOutputValues();
    // if sync mode ...
    if(asyncClock_step == -1) {
        // transfer input register values to real used operation registers (system clock delay 1 step in sync mode)
        TransferInputValues();
        // count prescaler, if sync. mode, e.g. timer clock is system clock
        TimerCounter();
    }

    // control pll state
    if(asyncClock_pll) {
        if(!asyncClock_plllock && (SystemClock::Instance().GetCurrentTime() >= asyncClock_locktime))
            asyncClock_plllock = true;
    }
    return 0;
}

void HWTimerTinyX5::TimerCounter(void) {
    // if prescaler mux gets a pulse ...
    if(PrescalerMux()) {
        // count pulse for timer
        counter++;
        // detect counter overflow (== OCRC + 1 or 0xff + 1!) ...
        if((counter > 0xff) ||
            (((cfg_mode != TMODE_NORMAL) || cfg_ctc) && ((counter - 1) == (unsigned char)ocrc_inout_val))) {
            counter = 0;
            // set TOV1 event but not in CTC mode!
            if((cfg_mode != TMODE_NORMAL) || !cfg_ctc)
                tov_internal_flag = true;
            // load compare values for OCR1A and OCR1B in PWM mode
            if(cfg_mode != TMODE_NORMAL) {
                ocra_compare = ocra_internal_val;
                ocrb_compare = ocrb_internal_val;
            }
            // trigger pin change (reset)
            ocra_unit.TimerEvent(false);
            ocrb_unit.TimerEvent(false);
        }
        // detect OCR1A event ...
        if(counter == ocra_compare) {
            // set OCF1A event
            tocra_internal_flag = true;
            // trigger pin change
            if(!(cfg_mode & TMODE_PWMA) || (ocra_compare < ocrc_inout_val))
                // no compare pin change, if PWM and OCRB == OCRC
                ocra_unit.TimerEvent(true);
        }
        // detect OCR1B event ...
        if(counter == ocrb_compare) {
            // set OCF1B event
            tocrb_internal_flag = true;
            // trigger pin change
            if(!(cfg_mode & TMODE_PWMB) || (ocrb_compare < ocrc_inout_val))
                // no compare pin change, if PWM and OCRB == OCRC
                ocrb_unit.TimerEvent(true);
        }
        counterTrace->change(counter);
    }
    // handle DeadTimePrescaler-Mux ...
    if(DeadTimePrescalerMux()) {
        // handle dead time counter
        ocra_unit.DTClockCycle();
        ocrb_unit.DTClockCycle();
    }
}

bool HWTimerTinyX5::PrescalerMux(void) {
    // count prescaler, free running
    prescaler++;
    if(prescaler == 0x4000) prescaler = 0; // prescaler with = 14 bit
    prescalerTrace->change(prescaler);
    // select prescaler tap
    switch(cfg_prescaler) {
    case 0: // no clock, counter stop
        return false;

    case 1: // every clock
        return true;

    case 2: // CKx2
        return (bool)((prescaler % 2) == 0);

    case 3: // CKx4
        return (bool)((prescaler % 4) == 0);

    case 4: // CKx8
        return (bool)((prescaler % 8) == 0);

    case 5: // CKx16
        return (bool)((prescaler % 16) == 0);

    case 6: // CKx32
        return (bool)((prescaler % 32) == 0);

    case 7: // CKx64
        return (bool)((prescaler % 64) == 0);

    case 8: // CKx128
        return (bool)((prescaler % 128) == 0);

    case 9: // CKx256
        return (bool)((prescaler % 256) == 0);

    case 10: // CKx512
        return (bool)((prescaler % 512) == 0);

    case 11: // CKx1024
        return (bool)((prescaler % 1024) == 0);

    case 12: // CKx2048
        return (bool)((prescaler % 2048) == 0);

    case 13: // CKx4096
        return (bool)((prescaler % 4096) == 0);

    case 14: // CKx8192
        return (bool)((prescaler % 8192) == 0);

    case 15: // CKx16384
        return (bool)((prescaler % 16384) == 0);
    }

    // should not happen
    return 0;
}

bool HWTimerTinyX5::DeadTimePrescalerMux(void) {
    // count prescaler, freerunning
    dtprescaler++;
    if(dtprescaler == 0x8) dtprescaler = 0; // dead time prescaler with = 3 bit
    dTPrescalerTrace->change(dtprescaler);
    // select dead time prescaler tap
    switch(cfg_dtprescaler) {
    case 0: // every clock
        return true;

    case 1: // CKx2
        return (bool)((dtprescaler % 2) == 0);

    case 2: // CKx4
        return (bool)((dtprescaler % 4) == 0);

    case 3: // CKx8
        return (bool)((dtprescaler % 8) == 0);
    }

    // should not happen
    return 0;
}

unsigned char HWTimerTinyX5::set_from_reg(const IOSpecialReg *reg, unsigned char nv) {
    if(reg == gtccrRegister) {
        // check prescaler reset bit [bit1]
        if((nv & 0x02) == 0x02) {
            nv &= ~0x02; // reset bit immediately
            prescaler = 0; // reset prescaler
        }
        gtccr_in_val = nv;
    } else if(reg == pllcsrRegister) {
        // reflect and control pll state
        bool pll = (nv & 0x02) == 0x02;
        if(asyncClock_pll) {
            if(!pll) {
                asyncClock_pll = false;
                asyncClock_plllock = false;
            }
        } else {
            if(pll) {
                asyncClock_pll = true;
                asyncClock_plllock = false;
                // delay about 100µs, uses rand() function, but this is ok here, we don't need
                // real random values, just a "jitter", results in a delay 100µs +- 1µs!
                // (but because data sheet dosn't tell anything about time deviation, this
                // is a assumption and to prove!)
                srand(time(NULL));
                unsigned long delay = 100000 + rand() % 2000 - 1000;
                asyncClock_locktime = SystemClock::Instance().GetCurrentTime() + delay;
            }
        }
        // get clock source for timer 1 [bit7 = LSM, bit2 = PCKE]
        asyncClock_lsm = (nv & 0x80) == 0x80;
        SetPrescalerClock((nv & 0x04) == 0x04);
    }
    return nv;
}

unsigned char HWTimerTinyX5::get_from_client(const IOSpecialReg *reg, unsigned char v) {
    if(reg == pllcsrRegister) {
        if(asyncClock_plllock)
            v |= 0x01;
        else
            v &= ~0x01;
    }
    if(reg == gtccrRegister) {
        v &= ~0x0c; // FOC1A and FOC1B will be read back as 0 in every case
    }
    return v;
}

void HWTimerTinyX5::SetPrescalerClock(bool pcke) {
    if(pcke) {
        // Async clock enabled?
        if(!asyncClock_async) {
            asyncClock_async = true;
            asyncClock_step = 0; // this disabled also timer calculation in CpuCycle() to be secure
            SystemClock::Instance().Add(this); // now the async clock is activated
        } else {
            if(asyncClock_lsm) asyncClock_step &= ~1; // on lsm async mode we take every second step
        }
    } else {
        // Sync clock enabled?
        if(asyncClock_step >= 0) {
            // switch to sync clock
            asyncClock_async = false; // disable is delayed to next call of Step()
        }
    }
}

void HWTimerTinyX5::TransferInputValues(void) {
    // settings from TCCR
    if(tccr_inout_val.ClockAndChanged()) {
        cfg_prescaler = tccr_inout_val & 0x0f;
        if((tccr_inout_val & 0x40) == 0x40)
            cfg_mode |= TMODE_PWMA;
        else
            cfg_mode &= ~TMODE_PWMA;
        cfg_com_a = (tccr_inout_val >> 4) & 0x3;
        ocra_unit.SetOCRMode((tccr_inout_val & 0x40) == 0x40, (tccr_inout_val >> 4) & 0x3);
        // set ctc mode, this is only possible, if counter counts till OCR1C value!
        cfg_ctc = (tccr_inout_val & 0x80) == 0x80;
    }
    // settings from GTCCR
    if(gtccr_in_val.ClockAndChanged()) {
        cfg_com_b = (gtccr_in_val >> 4) & 0x3;
        if((gtccr_in_val & 0x40) == 0x40)
            cfg_mode |= TMODE_PWMB;
        else
            cfg_mode &= ~TMODE_PWMB;
        ocrb_unit.SetOCRMode((gtccr_in_val & 0x40) == 0x40, (gtccr_in_val >> 4) & 0x3);
        if(gtccr_in_val & 0x04) {
            ocra_unit.ForceEvent();
            gtccr_in_val.MaskOutSync(0x04);
        }
        if(gtccr_in_val & 0x08) {
            ocrb_unit.ForceEvent();
            gtccr_in_val.MaskOutSync(0x08);
        }
    }
    // OCRA settings
    if(ocra_inout_val.ClockAndChanged()) {
        if(cfg_mode != TMODE_NORMAL)
            // take over value on timer overflow
            ocra_internal_val = ocra_inout_val;
        else
            // take over value immediately
            ocra_compare = ocra_inout_val;
    }
    // OCRB settings
    if(ocrb_inout_val.ClockAndChanged()) {
        if(cfg_mode != TMODE_NORMAL)
            // take over value on timer overflow
            ocrb_internal_val = ocrb_inout_val;
        else
            // take over value immediately
            ocrb_compare = ocrb_inout_val;
    }
    // OCRC settings
    ocrc_inout_val.ClockAndChanged();
    // timer update
    if(tcnt_set_flag) {
        counter = tcnt_in_val;
        tcnt_set_flag = false;
    }
    // settings from DTPS1
    cfg_dtprescaler = dtps1_inout_val & 0x03;
    // settings from DT1A
    if(dt1a_inout_val.ClockAndChanged())
        ocra_unit.SetDeadTime((dt1a_inout_val >> 4) & 0x0f, dt1a_inout_val & 0x0f);
    // settings from DT1B
    if(dt1b_inout_val.ClockAndChanged())
        ocrb_unit.SetDeadTime((dt1b_inout_val >> 4) & 0x0f, dt1b_inout_val & 0x0f);
}

void HWTimerTinyX5::TransferOutputValues(void) {
    // counter value TCNT
    if(asyncClock_step == -1)
        // in sync mode
        tcnt_out_val = counter;
    else
        // in async mode
        tcnt_out_val = tcnt_out_async_tmp;
    tcnt_out_async_tmp = counter;
    // TOV interrupt
    if(tov_internal_flag) {
        tov_internal_flag = false;
        timerOverflowInt->fireInterrupt();
    }
    // OCFxA interrupt
    if(tocra_internal_flag) {
        tocra_internal_flag = false;
        timerOCRAInt->fireInterrupt();
    }
    // OCFxB interrupt
    if(tocrb_internal_flag) {
        tocrb_internal_flag = false;
        timerOCRBInt->fireInterrupt();
    }
}

TimerTinyX5_OCR::TimerTinyX5_OCR(PinAtPort* pinOut, PinAtPort* pinOutInv) {
    outPin = pinOut;
    outPinInv = pinOutInv;

    Reset();
}

void TimerTinyX5_OCR::Reset() {
    ocrComMode = 0;
    ocrPWM = false;
    ocrOut = false;
    dtHigh = 0;
    dtLow = 0;
    dtCounter = 0;
}

void TimerTinyX5_OCR::DTClockCycle() {
    if(dtCounter > 0) {
        dtCounter--;
        if(dtCounter == 0) {
            // dead time arrived, set output or inverted output
            if(ocrOut)
                outPin->SetAlternatePort(ocrOut);
            else
                outPinInv->SetAlternatePort(!ocrOut);
        }
    }
}

void TimerTinyX5_OCR::SetOCRMode(bool isPWM, int comMode) {
    // if switch output on, then get value of out pin (for toggle function)
    if((ocrComMode == 0) && (comMode != 0)) {
        ocrOut = outPin->GetPort();
    }
    // enable/disable alternate port pin
    if(ocrComMode != comMode) {
        if(comMode > 0) {
            // connect normal pin
            outPin->SetUseAlternatePortIfDdrSet(true);
            outPin->SetAlternatePort(ocrOut);
            if(isPWM && comMode == 1) {
                // connect inverted pin
                outPinInv->SetUseAlternatePortIfDdrSet(true);
                outPinInv->SetAlternatePort(!ocrOut);
            }
        } else {
            // disconnect both pins
            outPin->SetUseAlternatePortIfDdrSet(false);
            outPinInv->SetUseAlternatePortIfDdrSet(false);
        }
    }
    // store values
    ocrComMode = comMode;
    ocrPWM = isPWM;
}

void TimerTinyX5_OCR::SetPWM(bool isCompareEvent) {
    bool out = ocrOut;
    if(ocrPWM) {
        // in PWM mode
        if(isCompareEvent) {
            switch(ocrComMode) {
            case 0:
                // output is disabled, do not change pin
                break;

            case 1:
            case 2:
                // clear on compare
                out = false;
                break;

            case 3:
                // set on compare
                out = true;
                break;
            }
        } else {
            switch(ocrComMode) {
            case 0:
                // output is disabled, do not change pin
                break;

            case 1:
            case 2:
                // set on overflow
                out = true;
                break;

            case 3:
                // clear on overflow
                out = false;
                break;
            }
        }
        SetDeadTime(out);
    } else {
        // in normal mode (only compare event)
        if(isCompareEvent) {
            switch(ocrComMode) {
            case 0:
                // output is disabled, do not change pin
                break;

            case 1:
                // toggle output
                if(out)
                    out = false;
                else
                    out = true;
                break;

            case 2:
                // clear pin
                out = false;
                break;

            case 3:
                // set pin
                out = true;
                break;
            }
            SetDeadTime(out);
        }
    }
}

void TimerTinyX5_OCR::SetDeadTime(bool pwmValue) {
    if((ocrComMode == 1) && ocrPWM) {
        // dead time generator is only active in mode 1 and PWM
        if(pwmValue && !ocrOut) {
            // rising edge
            if(dtHigh > 0)
                dtCounter = dtHigh + 1;
            else {
                // no dead time, set output immediately
                outPin->SetAlternatePort(pwmValue);
            }
            outPinInv->SetAlternatePort(!pwmValue);
        } else if(!pwmValue && ocrOut) {
            // falling edge
            if(dtLow > 0)
                dtCounter = dtLow + 1;
            else {
                // no dead time, set inverted output immediately
                outPinInv->SetAlternatePort(!pwmValue);
            }
            outPin->SetAlternatePort(pwmValue);
        }
    } else
        // set output immediately
        outPin->SetAlternatePort(pwmValue);
    ocrOut = pwmValue;
}

// EOF
