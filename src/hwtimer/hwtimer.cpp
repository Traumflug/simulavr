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
#include "hwtimer.h"
#include "hwtimer01irq.h"
#include "trace.h"
#include "helper.h"

using namespace std;

HWPrescaler::HWPrescaler(AvrDevice *core): Hardware(core) {
	core->AddToCycleList(this);
}

void HWTimer0::TimerCompareAfterCount() {
	if ((tcnt==1) ) { //overflow occured! when leaving 0
		timer01irq->AddFlagToTifr(timer01irq->TOV0);	//set TOV0 in TIFR
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
		timer01irq->AddFlagToTifr(timer01irq->TOV1);	//set TOV1 in TIFR
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
					pin_oc1a.SetAlternatePort(last_ocr1a);	//Toggle Pin
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
		timer01irq->AddFlagToTifr(timer01irq->OCF1A);	//set OCF1A in TIFR

	} //end of compare

	if (tcnt1==ocr1b) {
		if ( (tccr1a & 0x03) == 0x00) { //timer is not in pwm mode
			switch(tccr1a&(0x30)) {
				case 0x00:
					; //Nothing todo, settings menaged on other place :-)
					break;

				case 0x10:
					if (last_ocr1b==0) { last_ocr1b=1; } else { last_ocr1b=0;}
					pin_oc1b.SetAlternatePort(last_ocr1b);	//Toggle Pin
					break;

				case 0x20:
					last_ocr1b=0;
					pin_oc1b.SetAlternatePort(last_ocr1b);
					break;

				case 0x30:
					last_ocr1b=1;
					pin_oc1b.SetAlternatePort(last_ocr1b);
					break;

			}		//end of switch
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

		timer01irq->AddFlagToTifr(timer01irq->OCF1B);	//set OCF1B in TIFR
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
	    timer01irq->AddFlagToTifr(timer01irq->ICF1);	//set ICF1 in TIFR
	    icr1=tcnt1;	//Capture
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
	tccr1a=val;	//Setting for alternate pin ocr1a could be changed so test for this feature
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
