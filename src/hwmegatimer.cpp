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
#include "hwmegatimer0123irq.h"
#include <iostream>
#include "hwmegatimer.h"
#include "hwtimer.h"    //for prescaler
#include "trace.h"


const unsigned short HWMegaTimer1::topFFFF; //=0xffff;
const unsigned short HWMegaTimer1::topFF; //=0xff;
const unsigned short HWMegaTimer1::top1FF; //=0x1ff;
const unsigned short HWMegaTimer1::top3FF; //=0x3ff;
const unsigned short HWMegaTimer1::top0;

const unsigned char HWMegaTimer2::topFF; //=0xff;
const unsigned char HWMegaTimer2::top0;

const unsigned char HWMegaTimer0::topFF; //=0xff;
const unsigned char HWMegaTimer0::top0;

void HWMegaTimer0::OcrResetPin(unsigned char &ocr, bool &lastOcr, PinAtPort &pinOc, unsigned char ocrMode) {
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

bool HWMegaTimer0::OcrWork(unsigned char &ocr, bool &lastOcr, PinAtPort &pinOc, unsigned char ocrMode) {
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
						cout << "Timer1: Error Mode is reserved!" ;
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
						cout << "Timer1: Error Mode is reserved";
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

void HWMegaTimer0::TimerCompareAfterCount() {
	//cout << "Timer0: " << hex << (unsigned int)  tcnt << " ";
	//cout << "Timer0 tov: " << hex << (unsigned int) *pointerToTop << " ";

	//handling the tovr irq flag 
	if (tcnt==*tovCompare) {
		timer01irq->AddFlagToTifr(0x01);
	}
	//check for the ocr's 
	if( OcrWork(ocr, last_oc, pin_oc,((tccr>>4)&0x03)))timer01irq->AddFlagToTifr(0x02); 

	switch (timerMode) {
		//the only upcounting modes (normal, ctc, fast pwm)
		case 0: //normal
		case 2: //ctc
		case 3: //fast pwm
			if (*pointerToTop==tcnt) {
				tcnt=0;
				OcrResetPin(ocr, last_oc, pin_oc,((tccr>>4)&0x03));
			};
			break;

			//only upcounting in fast pwm, also reset the ocrx pins dependend on comnx0/1
		case 1:
			if (cntDir==0) { //downcounting
				if (0==tcnt) {
					cntDir=1; //count up now
				}
			} else { //upcounting
				if (*pointerToTop==tcnt) {
					cntDir=0; //count down now
				}
			}
			break;
	} //end of switch timerMode




}



HWMegaTimer0::HWMegaTimer0(AvrDevice *core, HWPrescaler *p, HWMegaTimer0123Irq *s, PinAtPort oc): Hardware(core), pin_oc(oc){
	core->AddToCycleList(this);
	prescaler=p;
	timer01irq= s;
	Reset();
}

void HWMegaTimer0::CheckForMode() {
	bool WGMn0=(tccr&0x40)!=0;
	bool WGMn1=(tccr&0x08)!=0;

	timerMode= (WGMn1 << 1) + WGMn0;

	switch (timerMode) {

		case 0:
			pointerToTop=&topFF;
			tovCompare=&topFF;
			break;
		case 1:
			pointerToTop=&topFF;
			tovCompare=&top0;
			break;
		case 2:
			pointerToTop=&ocr;
			tovCompare=&topFF;
			break;
		case 3:
			pointerToTop=&topFF;
			tovCompare=&topFF;
			break;
	} //end switch

	switch (timerMode) {
		case 0:
			switch(tccr&0x30) { //checking here for compare b
				case 0:
					pin_oc.SetUseAlternatePort(0); 
					pin_oc.SetDdr(0);
					break;

				case 0x10:
				case 0x20:
				case 0x30:
					pin_oc.SetUseAlternatePort(1);	
					pin_oc.SetDdr(1);	
					pin_oc.SetAlternatePort(last_oc);
					break;
			} // end of switch
			break;

		default: //all other modes are equal
			switch(tccr&0x30) {
				case 0x00:
					pin_oc.SetUseAlternatePortIfDdrSet(0); 
					break;

				case 0x10:
				case 0x20:
				case 0x30:
					pin_oc.SetUseAlternatePortIfDdrSet(1);	
					pin_oc.SetAlternatePort(last_oc);
					break;
			}

			break;

	} //end of switch timerMode
}

unsigned int HWMegaTimer0::CpuCycle(){
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

		case CK32:
			if ((prescaler->GetValue()%32)==0){
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
//--------------------------------------------------------------------------------------
void HWMegaTimer2::OcrResetPin(unsigned char &ocr, bool &lastOcr, PinAtPort &pinOc, unsigned char ocrMode) {
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

bool HWMegaTimer2::OcrWork(unsigned char &ocr, bool &lastOcr, PinAtPort &pinOc, unsigned char ocrMode) {
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
						cout << "Timer1: Error Mode is reserved!" ;
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
						cout << "Timer1: Error Mode is reserved";
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

void HWMegaTimer2::TimerCompareAfterCount() {

	//check for the ocr's 
	if( OcrWork(ocr, last_oc, pin_oc,((tccr>>4)&0x03)))timer01irq->AddFlagToTifr(0x80); 

	//handling the tovr irq flag 
	if (tcnt==*tovCompare) {
		timer01irq->AddFlagToTifr(0x40);
	}

	switch (timerMode) {
		//the only upcounting modes (normal, ctc, fast pwm)
		case 0: //normal
		case 2: //ctc
		case 3: //fast pwm
			if (*pointerToTop==tcnt) {
				tcnt=0;
				OcrResetPin(ocr, last_oc, pin_oc,((tccr>>4)&0x03));
			};
			break;

			//only upcounting in fast pwm, also reset the ocrx pins dependend on comnx0/1
		case 1:
			if (cntDir==0) { //downcounting
				if (0==tcnt) {
					cntDir=1; //count up now
				}
			} else { //upcounting
				if (*pointerToTop==tcnt) {
					cntDir=0; //count down now
				}
			}
			break;
	} //end of switch timerMode




}



HWMegaTimer2::HWMegaTimer2(AvrDevice *core, HWPrescaler *p, HWMegaTimer0123Irq *s, PinAtPort pi, PinAtPort oc): Hardware(core),pin_t0(pi), pin_oc(oc){
	core->AddToCycleList(this);
	prescaler=p;
	timer01irq= s;
	Reset();
}

void HWMegaTimer2::CheckForMode() {
	bool WGMn0=(tccr&0x40)!=0;
	bool WGMn1=(tccr&0x08)!=0;

	timerMode= (WGMn1 << 1) + WGMn0;

	switch (timerMode) {

		case 0:
			pointerToTop=&topFF;
			tovCompare=&topFF;
			break;
		case 1:
			pointerToTop=&topFF;
			tovCompare=&top0;
			break;
		case 2:
			pointerToTop=&ocr;
			tovCompare=&topFF;
			break;
		case 3:
			pointerToTop=&topFF;
			tovCompare=&topFF;
			break;
	} //end switch

	switch (timerMode) {
		case 0:
			switch(tccr&0x30) { //checking here for compare b
				case 0:
					pin_oc.SetUseAlternatePort(0); 
					pin_oc.SetDdr(0);
					break;

				case 0x10:
				case 0x20:
				case 0x30:
					pin_oc.SetUseAlternatePort(1);	
					pin_oc.SetDdr(1);	
					pin_oc.SetAlternatePort(last_oc);
					break;
			} // end of switch
			break;

		default: //all other modes are equal
			switch(tccr&0x30) {
				case 0x00:
					pin_oc.SetUseAlternatePortIfDdrSet(0); 
					break;

				case 0x10:
				case 0x20:
				case 0x30:
					pin_oc.SetUseAlternatePortIfDdrSet(1);	
					pin_oc.SetAlternatePort(last_oc);
					break;
			}

			break;

	} //end of switch timerMode
}

unsigned int HWMegaTimer2::CpuCycle(){
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
//--------------------------------------------------------------------------------------

HWMegaTimer1::HWMegaTimer1
(AvrDevice *core, HWPrescaler *p, HWMegaTimer0123Irq *s, bool isT1, PinAtPort t1, PinAtPort oca, PinAtPort ocb, PinAtPort occ) :
Hardware(core), isTimer1(isT1), pin_t1(t1), pin_oc1a(oca), pin_oc1b(ocb), pin_oc1c(occ) { 
	core->AddToCycleList(this);
	cntDir=1;
	prescaler=p, 
	timer01irq=s;
	Reset();
}


void HWMegaTimer1::Reset()
{

	tccr1a=0;
	tccr1b=0;
	tcnt1=0;
	ocr1a=0;
	ocr1b=0;
	icr1=0;

	last_ocr1a=0;
	last_ocr1b=0;
	last_ocr1c=0;
	pointerToTop=&topFFFF;
}

void HWMegaTimer1::OcrResetPin(unsigned short &ocr, bool &lastOcr, PinAtPort &pinOc, unsigned char ocrMode) {
	//no mode switch here, while this function is only called in fast pwm mode :-)
	bool WGMn3=(tccr1b&0x10)>>4;

	switch (ocrMode) {
		case 0:
			//disconnected, nothing to do
			break;

		case 1:
			if (WGMn3!=0) {
				lastOcr=!lastOcr;
			}
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

bool HWMegaTimer1::OcrWork(unsigned short &ocr, bool &lastOcr, PinAtPort &pinOc, unsigned char ocrMode) {
	bool WGMn3=(tccr1b&0x10)>>4;

	if (ocr==tcnt1) {
		switch (timerMode) {
			//normal mode
			case 0:
				// and the ctc modes, mode 2,3 makes no sense while the pins are hold at low or high then :-)
			case 4:
			case 12:
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

				// Compare Output Mode, Phase Correct and Phase and Frequency Correct PWM

			case 1:
			case 2:
			case 3:
			case 8:
			case 9:
			case 10:
			case 11:

				switch (ocrMode) {
					case 0:
						//pin has gpio functionality, nothing to handle here
						break;
					case 1:
						if (WGMn3!=0) { //is not gpio
							lastOcr=!lastOcr; //Toggle
						}
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


				// Compare Output Mode, Fast PWM
			case 5:
			case 6:
			case 7:
			case 14:
			case 15:
				switch (ocrMode) {
					case 0:
						//disconnected, nothing to do
						break;

					case 1:
						if (WGMn3!=0) {
							lastOcr=!lastOcr;
						}
						break;

					case 2:
						lastOcr=0;	//set on top not handled yet, TODO
						break;

					case 3:
						lastOcr=1;	//clear on top not handled yet, TODO
						break;
				}
				break;


		} //end of switch timerMode
		pinOc.SetAlternatePort(lastOcr);	
		return 1; //compare was equal->set irq
	} //end of (tcnt1==ocr) 
	return 0; //compare was not equal, so set no irq flag for ocrnx	
}

//compares
void HWMegaTimer1::TimerCompareAfterCount() {

	//handling the tovr irq flag 
	if (tcnt1==*tovCompare) {
		if (isTimer1) { 
			timer01irq->AddFlagToTifr(0x04);
		} else {
			timer01irq->AddFlagToEtifr(0x04);
		}
	}

	//check for the ocr's 
	if (isTimer1) { //remove that very ugly stuff here
		if( OcrWork(ocr1a, last_ocr1a, pin_oc1a,((tccr1a>>6)&0x03)))timer01irq->AddFlagToTifr(0x10); 
		if( OcrWork(ocr1b, last_ocr1b, pin_oc1b,((tccr1a>>4)&0x03)))timer01irq->AddFlagToTifr(0x08); 
		if( OcrWork(ocr1c, last_ocr1c, pin_oc1c,((tccr1a>>2)&0x03)))timer01irq->AddFlagToEtifr(0x01);
	} else { //this is timer 3 !!ugly at all, TODO
		if( OcrWork(ocr1a, last_ocr1a, pin_oc1a,((tccr1a>>6)&0x03)))timer01irq->AddFlagToEtifr(0x10); 
		if( OcrWork(ocr1b, last_ocr1b, pin_oc1b,((tccr1a>>4)&0x03)))timer01irq->AddFlagToEtifr(0x08); 
		if( OcrWork(ocr1c, last_ocr1c, pin_oc1c,((tccr1a>>2)&0x03)))timer01irq->AddFlagToEtifr(0x02);
	}

	switch (timerMode) {
		//the only upcounting modes (normal, ctc, fast pwm)
		case 0:
		case 4:
		case 12:
			//also reset the ocrx pins in normal and ctc modes !
			/*
			if (*pointerToTop==tcnt1) {
			tcnt1=0;
			};
			break;
			*/

			//only upcounting in fast pwm, also reset the ocrx pins dependend on comnx0/1
		case 5:
		case 6:
		case 7:
		case 14:
		case 15:
			if (*pointerToTop==tcnt1) {
				tcnt1=0;
				OcrResetPin(ocr1a, last_ocr1a, pin_oc1a,((tccr1a>>6)&0x03));
				OcrResetPin(ocr1b, last_ocr1b, pin_oc1b,((tccr1a>>4)&0x03));
				OcrResetPin(ocr1c, last_ocr1c, pin_oc1c,((tccr1a>>2)&0x03));
			}
			break;



		case 1:
		case 2:
		case 3:
		case 8:
		case 9:
		case 10:
		case 11:
			if (cntDir==0) { //downcounting
				if (0==tcnt1) {
					cntDir=1; //count up now
				}
			} else { //upcounting
				if (*pointerToTop==tcnt1) {
					cntDir=0; //count down now
				}
			}
			break;
		case 13:
			cout << "Timer Mode 13 not allowed!";
			break;

	} //end of switch timerMode




	if (tcnt1==icr1) {
		switch (timerMode) {
			case 8:
			case 10:
			case 12:
			case 14:
				//set icf1 flag while icr is used as top TODO
				if (isTimer1) {
					timer01irq->AddFlagToTifr(0x20);
				} else {
					timer01irq->AddFlagToEtifr(0x04);
				}
				break;
		} //end of switch timer mode
	}
}

unsigned int HWMegaTimer1::CpuCycle(){
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
	//
	if ( icp!=icp_old) { //pin change for capture detected
		inputCaptureNoiseCnt++;
		if ( tccr1b&0x80==0 | (inputCaptureNoiseCnt>=4)) {
			inputCaptureNoiseCnt=0;
			icp_old=icp;

			//Edge detection
			if (( icp ^ (tccr1b&0x40)>>6)==1) {
				//edge OK
				timer01irq->AddFlagToTifr(0x08);	//set ICF1 in TIFR
				icr1=tcnt1;	//Capture


			}	//end of edge detection
		} // end of noise canceler 
	} // end of pin change 
	return 0;
}

void HWMegaTimer1::CheckForMode() {
	bool WGMn3=(tccr1b&0x10)!=0;
	bool WGMn2=(tccr1b&0x08)!=0;
	bool WGMn1=(tccr1a&0x02)!=0;
	bool WGMn0=(tccr1a&0x01)!=0;
	//bool isInPwmMode = WGMn3 || WGMn2 || WGMn1 || WGMn0 ; //if any of bits WGMn3..2 or WGMn1..0  is set



	timerMode= (WGMn3<<3) + (WGMn2 <<2) + (WGMn1 << 1) + WGMn0;

	switch (timerMode) {

		case 0:
			pointerToTop=&topFFFF;
			tovCompare=&topFFFF;
			break;
		case 1:
			pointerToTop=&topFF;
			tovCompare=&top0;
			break;
		case 2:
			pointerToTop=&top1FF;
			tovCompare=&top0;
			break;
		case 3:
			pointerToTop=&top3FF;
			tovCompare=&top0;
			break;
		case 4:
			pointerToTop=&ocr1a;
			tovCompare=&topFFFF;
			break;
		case 5:
			pointerToTop=&topFF;
			tovCompare=pointerToTop;
			break;
		case 6:
			pointerToTop=&top1FF;
			tovCompare=pointerToTop;
			break;
		case 7:
			pointerToTop=&top3FF;
			tovCompare=pointerToTop;
			break;
		case 8:
			pointerToTop=&icr1;
			tovCompare=&top0;
			break;
		case 9:
			pointerToTop=&ocr1a;
			tovCompare=&top0;
			break;
		case 10:
			pointerToTop=&icr1;
			tovCompare=&top0;
			break;
		case 11:
			pointerToTop=&ocr1a;
			tovCompare=&top0;
			break;
		case 12:
			pointerToTop=&icr1;
			tovCompare=&topFFFF;
			break;
		case 13:
			cout << "Illegal Timer Mode seleted!";
			break;
		case 14:
			pointerToTop=&icr1;
			tovCompare=pointerToTop;
			break;
		case 15:
			pointerToTop=&ocr1a;
			tovCompare=pointerToTop;
			break;
	} //end switch

	switch (timerMode) {
		case 0:
		case 4:
		case 12:

			switch(tccr1a&0xc0) { //checking here for compare a
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

			switch(tccr1a&0x30) { //checking here for compare b
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

			switch(tccr1a&0x0c) { //checking here for compare c
				case 0:
					pin_oc1c.SetUseAlternatePort(0); 
					pin_oc1c.SetDdr(0);
					break;

				case 0x04:
				case 0x08:
				case 0x0c:
					pin_oc1c.SetUseAlternatePort(1);	
					pin_oc1c.SetDdr(1);	
					pin_oc1c.SetAlternatePort(last_ocr1c);
					break;
			} // end of switch


			break;

		default: //all other modes are equal

			if (WGMn3) { //WGMn3 is set

				switch(tccr1a&0xc0) {
					case 0x00:
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
					case 0x00:
						pin_oc1b.SetUseAlternatePortIfDdrSet(0); 
						break;

					case 0x10:
					case 0x20:
					case 0x30:
						pin_oc1b.SetUseAlternatePortIfDdrSet(1);	
						pin_oc1b.SetAlternatePort(last_ocr1b);
						break;
				}

				switch(tccr1a&0x0c) {
					case 0x00:
						pin_oc1c.SetUseAlternatePortIfDdrSet(0); 
						break;

					case 0x04:
					case 0x08:
					case 0x0c:
						pin_oc1c.SetUseAlternatePortIfDdrSet(1);	
						pin_oc1c.SetAlternatePort(last_ocr1c);
						break;
				}

			} else { //WGMn3==0


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
						pin_oc1c.SetUseAlternatePortIfDdrSet(0); 
						break;

					case 0x20:
					case 0x30:
						pin_oc1b.SetUseAlternatePortIfDdrSet(1);	
						pin_oc1b.SetAlternatePort(last_ocr1b);
						break;
				}

				switch(tccr1a&0x0c) {
					case 0x00:
					case 0x04:
						pin_oc1c.SetUseAlternatePortIfDdrSet(0); 
						break;

					case 0x08:
					case 0x0c:
						pin_oc1c.SetUseAlternatePortIfDdrSet(1);	
						pin_oc1c.SetAlternatePort(last_ocr1c);
						break;
				}
			} // end of WGMn3
			break;

	} //end of switch timerMode
}


unsigned char RWTccraM::operator=(unsigned char val) { trioaccess("TccraM",val);timer1->SetTccr1a(val); return val; }
unsigned char RWTccrbM::operator=(unsigned char val) { trioaccess("TccrbM",val);timer1->SetTccr1b(val); return val; }
unsigned char RWTccrcM::operator=(unsigned char val) { trioaccess("TccrcM",val);timer1->SetTccr1c(val); return val; }
unsigned char RWTcnthM::operator=(unsigned char val) { trioaccess("TcnthM",val);timer1->SetTcnt1h(val); return val; }
unsigned char RWTcntlM::operator=(unsigned char val) { trioaccess("TcntlM",val);timer1->SetTcnt1l(val); return val; }
unsigned char RWOcrahM::operator=(unsigned char val) { trioaccess("OcrahM",val);timer1->SetOcr1ah(val); return val; }
unsigned char RWOcralM::operator=(unsigned char val) { trioaccess("OcralM",val);timer1->SetOcr1al(val); return val; }
unsigned char RWOcrbhM::operator=(unsigned char val) { trioaccess("OcrbhM",val);timer1->SetOcr1bh(val); return val; }
unsigned char RWOcrblM::operator=(unsigned char val) { trioaccess("OcrblM",val);timer1->SetOcr1bl(val); return val; }
unsigned char RWOcrchM::operator=(unsigned char val) { trioaccess("OcrchM",val);timer1->SetOcr1ch(val); return val; }
unsigned char RWOcrclM::operator=(unsigned char val) { trioaccess("OcrclM",val);timer1->SetOcr1cl(val); return val; }
unsigned char RWIcrhM::operator=(unsigned char val) { trioaccess("IcrhM",val);cout << "Write To Read Only Register ICR1H";  return val; }
unsigned char RWIcrlM::operator=(unsigned char val) { trioaccess("IcrlM",val);cout << "Write To Read Only Register ICR1L";  return val; }
unsigned char RWEtimskM::operator=(unsigned char val) { trioaccess("EtimskM",val);hwTimer01Irq->SetTimsk(val);  return val; }
unsigned char RWEtifrM::operator=(unsigned char val) { trioaccess("EtifrM",val);hwTimer01Irq->SetTifr(val);  return val; }
unsigned char RWTimskM::operator=(unsigned char val) { trioaccess("TimskM",val);hwTimer01Irq->SetTimsk(val);  return val; }
unsigned char RWTifrM::operator=(unsigned char val) { trioaccess("TifrM",val);hwTimer01Irq->SetTifr(val);  return val; }
unsigned char RWTcnt0::operator=(unsigned char val) { trioaccess("Tcnt0",val);timer0->SetTcnt(val);  return val; }
unsigned char RWOcr0::operator=(unsigned char val) { trioaccess("Ocr0",val);timer0->SetOcr(val);  return val; }
unsigned char RWTccr0::operator=(unsigned char val) { trioaccess("Tccr0",val);timer0->SetTccr(val);  return val; }
unsigned char RWTccr2::operator=(unsigned char val) { trioaccess("Tccr2",val);timer0->SetTccr(val);  return val; }
unsigned char RWTcnt2::operator=(unsigned char val) { trioaccess("Tcnt2",val);timer0->SetTcnt(val);  return val; }
unsigned char RWOcr2::operator=(unsigned char val) { trioaccess("Ocr2",val);timer0->SetOcr(val);  return val; }



RWTccr0::operator unsigned char() const { return timer0->GetTccr();}
RWTcnt0::operator unsigned char() const { return timer0->GetTcnt();}
RWOcr0::operator unsigned char() const { return timer0->GetOcr();}

RWTccr2::operator unsigned char() const { return timer0->GetTccr();}
RWTcnt2::operator unsigned char() const { return timer0->GetTcnt();}
RWOcr2::operator unsigned char() const { return timer0->GetOcr();}


RWTccraM::operator unsigned char() const { return timer1->GetTccr1a();}
RWTccrbM::operator unsigned char() const { return timer1->GetTccr1b();}
RWTccrcM::operator unsigned char() const { return timer1->GetTccr1c();}
RWTcnthM::operator unsigned char() const { return timer1->GetTcnt1h();}
RWTcntlM::operator unsigned char() const { return timer1->GetTcnt1l();}
RWOcrahM::operator unsigned char() const { return timer1->GetOcr1ah();}
RWOcralM::operator unsigned char() const { return timer1->GetOcr1al();}
RWOcrbhM::operator unsigned char() const { return timer1->GetOcr1bh();}
RWOcrblM::operator unsigned char() const { return timer1->GetOcr1bl();}
RWOcrchM::operator unsigned char() const { return timer1->GetOcr1ch();}
RWOcrclM::operator unsigned char() const { return timer1->GetOcr1cl();}
RWIcrhM::operator unsigned char() const {  return timer1->GetIcr1h();}
RWIcrlM::operator unsigned char() const {  return timer1->GetIcr1l();}

RWTimskM::operator unsigned char() const { return hwTimer01Irq->GetTimsk(); }
RWTifrM::operator unsigned char() const { return hwTimer01Irq->GetTifr(); } 
RWEtimskM::operator unsigned char() const { return hwTimer01Irq->GetTimsk(); }
RWEtifrM::operator unsigned char() const { return hwTimer01Irq->GetTifr(); } 
