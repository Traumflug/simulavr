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
#include "hardware.h"
#include "avrdevice.h"
#include "hwmegax8timerirq.h"
#include <iostream>
#include "hwmegax8timer.h"
#include "hwtimer.h"    //for prescaler
#include "trace.h"

using namespace std;

const unsigned char HWMegaX8Timer0::topFF; //=0xff;
const unsigned char HWMegaX8Timer0::top0;

void HWMegaX8Timer0::OcrResetPin(unsigned char &ocr, bool &lastOcr, PinAtPort &pinOc, unsigned char ocrMode) {
	//no mode switch here, while this function is only called in fast pwm mode :-)

	switch (ocrMode) {
		case 0:
			//disconnected, nothing to do
			break;

		case 1:
			lastOcr=!lastOcr;
			break;

		case 2:
			lastOcr=1;	
			break;

		case 3:
			lastOcr=0;
			break;
	}
	pinOc.SetAlternatePort(lastOcr);	
}

#define CS00 0
#define CS01 1
#define CS02 2


void HWMegaX8Timer0::SetTccra(unsigned char val)
{
    tccra=val; 
	CheckForMode();
}

void HWMegaX8Timer0::SetTccrb(unsigned char val)
{
    unsigned char cksourceold=tccrb&((1<<CS02)|(1<<CS01)|(1<<CS00));

    tccrb=val; 
	CheckForMode();

    unsigned char cksource= tccrb&((1<<CS02)|(1<<CS01)|(1<<CS00));

    if (cksource) { //switch of cpu cycle, counter is stopped 
	core->AddToCycleList(this);
    } else {
	core->RemoveFromCycleList(this);
    }
}

bool HWMegaX8Timer0::OcrWork(unsigned char &ocr, bool &lastOcr, PinAtPort &pinOc, unsigned char ocrMode) {
	if (ocr==tcnt) {
		switch (timerMode) {
			//normal mode
			case 0: // normal
			case 2: // CTC 
				switch (ocrMode) {
					case 0:
						//Nothing to do, pins ocrx connected to gpio
						break;

					case 1:
						//Toggling
						if (lastOcr==0) { lastOcr=1; } else { lastOcr=0;}
						break;

					case 2:
						//Clear on Compare Match
						lastOcr=0;
						break;

					case 3:
						//Set on Compare Match
						lastOcr=1;
						break;
				}
				break;

			case 1: // Phase Correct PWM
				switch (ocrMode) {
					case 0:
						//pin has gpio functionality, nothing to handle here
						break;
					case 1:
						cerr << "Timer1: Error Mode is reserved!" ;
						break;

					case 2:
						if (cntDir==1) { //upcounting
							lastOcr=0;
						} else {
							lastOcr=1;
						}
						break;
					case 3:
						if (cntDir==1) { //upcounting
							lastOcr=1;
						} else {
							lastOcr=0;
						}
						break;
				} // end of switch ocrMode
				break;


			case 3: //Fast PWM
				switch (ocrMode) {
					case 0:
						//disconnected, nothing to do
						break;

					case 1:
						cerr << "Timer1: Error Mode is reserved";
						break;

					case 2:
						lastOcr=0;	
						break;

					case 3:
						lastOcr=1;	
						break;
				}
				break;


		} //end of switch timerMode
		pinOc.SetAlternatePort(lastOcr);	
		return 1; //compare was equal->set irq
	} //end of (tcnt1==ocr) 
	return 0; //compare was not equal, so set no irq flag for ocrnx	
}

void HWMegaX8Timer0::TimerCompareAfterCount() {
	//handling the tovr irq flag 
	if (tcnt==*tovCompare) {
		timerIrq->AddFlagToTifr(0x01);	// TOV0
	}

	//check for the ocr's 
	if( OcrWork(ocra, last_oca, pin_oca,((tccra>>6)&0x03)))timerIrq->AddFlagToTifr(0x02);

	if( OcrWork(ocrb, last_ocb, pin_ocb,((tccra>>4)&0x03)))timerIrq->AddFlagToTifr(0x04); 

	switch (timerMode) {
		//the only upcounting modes (normal, ctc, fast pwm)
		case 0: //normal
		case 2: //ctc
		case 3: //fast pwm
			if (*pointerToTopA==tcnt) {
				tcnt=0;
				OcrResetPin(ocra, last_oca, pin_oca,((tccra>>6)&0x03));
			};
			if (*pointerToTopB==tcnt) {
				tcnt=0;
				OcrResetPin(ocrb, last_ocb, pin_ocb,((tccra>>4)&0x03));
			};
			break;

			//only upcounting in fast pwm, also reset the ocrx pins dependend on comnx0/1
		case 1:
			if (cntDir==0) { //downcounting
				if (0==tcnt) {
					cntDir=1; //count up now
				}
			} else { //upcounting
				if (*pointerToTopA==tcnt) {
					cntDir=0; //count down now
				}
				if (*pointerToTopB==tcnt) {
					cntDir=0; //count down now
				}
			}
			break;
	} //end of switch timerMode

}



HWMegaX8Timer0::HWMegaX8Timer0(AvrDevice *_c, HWPrescaler *p, HWMegaX8TimerIrq *s, PinAtPort oca, PinAtPort ocb):
	Hardware(_c), core(_c), pin_oca(oca), pin_ocb(ocb),
    tcnt_reg(core, "TIMER0.TCNT",
             this, &HWMegaX8Timer0::GetTcnt, &HWMegaX8Timer0::SetTcnt),
    ocra_reg(core, "TIMER0.OCRA",
             this, &HWMegaX8Timer0::GetOcra, &HWMegaX8Timer0::SetOcra),
    ocrb_reg(core, "TIMER0.OCRB",
             this, &HWMegaX8Timer0::GetOcrb, &HWMegaX8Timer0::SetOcrb),
    tccra_reg(core, "TIMER0.TCCRA",
              this, &HWMegaX8Timer0::GetTccra, &HWMegaX8Timer0::SetTccra),
    tccrb_reg(core, "TIMER0.TCCRB",
              this, &HWMegaX8Timer0::GetTccrb, &HWMegaX8Timer0::SetTccrb) {
	//_c->AddToCycleList(this);
	prescaler=p;
	timerIrq= s;
	Reset();
}

void HWMegaX8Timer0::CheckForMode() {
	bool WGMn0=(tccra&0x01)!=0;
	bool WGMn1=(tccra&0x02)!=0;
	bool WGMn2=(tccrb&0x08)!=0;

	timerMode= (WGMn2 << 2) + (WGMn1 << 1) + WGMn0;

	switch (timerMode) {

		case 0:
			pointerToTopA=&topFF;
			pointerToTopB=&topFF;
			tovCompare=&topFF;
			break;
		case 1:
			pointerToTopA=&topFF;
			pointerToTopB=&topFF;
			tovCompare=&top0;
			break;
		case 2:
			pointerToTopA=&ocra;
			pointerToTopB=&ocrb;
			tovCompare=&topFF;
			break;
		case 3:
			pointerToTopA=&topFF;
			pointerToTopB=&topFF;
			tovCompare=&topFF;
			break;
		default:
			pointerToTopA=&topFF;
			pointerToTopB=&topFF;
			tovCompare=&topFF;
			cerr << "Unimplemented timer mode" << endl;
			break;
	} //end switch

	switch (timerMode) {
		case 0:
			switch(tccra&0xC0) { //checking here for compare A
				case 0:	// Normal
					pin_oca.SetUseAlternatePort(0); 
					pin_oca.SetDdr(0);
					break;

				case 0x10:	// Toggle
				case 0x20:	// Clear on compare match
				case 0x30:	// Set on compare match
					pin_oca.SetUseAlternatePort(1);	
					pin_oca.SetDdr(1);	
					pin_oca.SetAlternatePort(last_oca);
					break;
			} // end of switch

			switch(tccra&0x30) { //checking here for compare b
				case 0:	// Normal
					pin_ocb.SetUseAlternatePort(0); 
					pin_ocb.SetDdr(0);
					break;

				case 0x10:	// Toggle
				case 0x20:	// Clear on compare match
				case 0x30:	// Set on compare match
					pin_ocb.SetUseAlternatePort(1);	
					pin_ocb.SetDdr(1);	
					pin_ocb.SetAlternatePort(last_ocb);
					break;
			} // end of switch
			break;

		default: //all other modes are equal
			switch(tccra&0xC0) {
				case 0x00:
					pin_oca.SetUseAlternatePortIfDdrSet(0); 
					break;

				case 0x10:
				case 0x20:
				case 0x30:
					pin_oca.SetUseAlternatePortIfDdrSet(1);	
					pin_oca.SetAlternatePort(last_oca);
					break;
			}
			switch(tccra&0x30) {
				case 0x00:
					pin_ocb.SetUseAlternatePortIfDdrSet(0); 
					break;

				case 0x10:
				case 0x20:
				case 0x30:
					pin_ocb.SetUseAlternatePortIfDdrSet(1);	
					pin_ocb.SetAlternatePort(last_ocb);
					break;
			}

			break;

	} //end of switch timerMode
}

unsigned int HWMegaX8Timer0::CpuCycle(){
	switch (tccrb & 0x07) {
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

		case CK32:
			if ((prescaler->GetValue()%32)==0){
				tcnt++;
				TimerCompareAfterCount();
			}
			break;

		case CK64:
			if ((prescaler->GetValue()%64)==0){
				++tcnt;
				TimerCompareAfterCount();
			}
			break;

		case CK128:
			if ((prescaler->GetValue()%128)==0){
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

	}

	return 0;
}
