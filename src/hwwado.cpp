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

#include "hwwado.h"
#include "avrdevice.h"
#include "systemclock.h"

#define WDTOE 0x10
#define WDE 0x08


void HWWado::SetWdtcr(unsigned char val) {
	unsigned char oldWDTOE= wdtcr & WDTOE;
	unsigned char newWDE= val   & WDE;

	if ( newWDE != 0) { 			//enable the wado allways allowed
		wdtcr=val;
	} else {  						//unset the wado
		if (oldWDTOE !=0) { 		//WDTOE was set, 
			wdtcr=val;
		}
	} 

	if ( (val & WDTOE ) != 0) {
		cntWde=4;
	}

} 

unsigned int HWWado::CpuCycle() {
	if ( cntWde > 0) {
		cntWde--;

	}

	if (cntWde==0) wdtcr&=(0xff-WDTOE); //clear WDTOE after 4 cpu cycles

	if ((( wdtcr& WDE )!= 0 ) && (timeOutAt < SystemClock::Instance().GetCurrentTime() )) {
		core->Reset();
	}


	return 0;
}

HWWado::HWWado(AvrDevice *c):
    Hardware(c),
    TraceValueRegister(c, "WADO"),
    core(c),
    wdtcr_reg(this, "WDTCR",
              this, &HWWado::GetWdtcr, &HWWado::SetWdtcr) {
	core->AddToCycleList(this);
	Reset();
}

void HWWado::Reset() {
	timeOutAt=0;
	wdtcr=0;
}


void HWWado::Wdr() {
	SystemClockOffset currentTime= SystemClock::Instance().GetCurrentTime(); 
	switch ( wdtcr& 0x7) {
		case 0:
			timeOutAt= currentTime+ 47000000; //47ms
			break;

		case 1:
			timeOutAt= currentTime+ 94000000; //94ms
			break;

		case 2:
			timeOutAt= currentTime+ 190000000; //190 ms
			break;

		case 3:
			timeOutAt= currentTime+ 380000000; //380 ms
			break;

		case 4:
			timeOutAt= currentTime+ 750000000; //750 ms
			break;

		case 5:
			timeOutAt= currentTime+ 1500000000; //1.5 s
			break;
			
		case 6:
			timeOutAt= currentTime+ 3000000000ULL; //3 s
			break;
		
		case 7:
			timeOutAt= currentTime+ 6000000000ULL; //6 s
			break;

	}
}
