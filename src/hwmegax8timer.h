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
#ifndef HWMEGAX8TIMER
#define HWMEGAX8TIMER

#include "hardware.h"
#include "pinatport.h"
#include "rwmem.h"

class HWIrqSystem;
class HWMegaX8TimerIrq;
class HWPrescaler;

class HWMegaX8Timer0: public Hardware {
	protected:
		unsigned char tccra;
		unsigned char tccrb;
		unsigned char tcnt;
		unsigned char ocra;
		unsigned char ocrb;

        AvrDevice *core;
		HWPrescaler *prescaler;
		HWMegaX8TimerIrq *timerIrq;
		PinAtPort pin_oca;
		PinAtPort pin_ocb;


		bool t0_old; 	//last state of external t0 pin
		bool last_oca;
		bool last_ocb;
		bool cntDir;

		static const unsigned char top0=0x0000;
		static const unsigned char topFF=0xff;

		const unsigned char *pointerToTopA;
		const unsigned char *pointerToTopB;
		const unsigned char *tovCompare;

		unsigned char timerMode;

		enum {
			STOP=0,
			CK,
			CK8,
			CK32,
			CK64,
			CK128,
			CK256,
			CK1024,
		} T_prescaler;


		void TimerCompareAfterCount();
		void CheckForMode();
		bool OcrWork( unsigned char &ocr, bool &lastOcr, PinAtPort &pinOc, unsigned char ocrMode);
		void OcrResetPin(unsigned char &ocr, bool &lastOcr, PinAtPort &pinOc, unsigned char ocrMode);

	public:
		HWMegaX8Timer0(AvrDevice *core, HWPrescaler *p, HWMegaX8TimerIrq *s, PinAtPort oca, PinAtPort ocb);

		void Reset() {
			SetTccra(0);
			SetTccrb(0);
			SetTcnt(0);
			t0_old=0;

		}
		virtual unsigned int CpuCycle();
		void SetTccra(unsigned char val);
		void SetTccrb(unsigned char val);
		void SetTcnt(unsigned char val) { tcnt=val; }
		void SetOcra(unsigned char val) {ocra=val; } //double buffering is missing TODO
		void SetOcrb(unsigned char val) {ocrb=val; } //double buffering is missing TODO
		unsigned char GetTccra() { return tccra; }
		unsigned char GetTccrb() { return tccrb; }
		unsigned char GetTcnt() { return tcnt; }
		unsigned char GetOcra() { return ocra; }
		unsigned char GetOcrb() { return ocrb; }

        IOReg<HWMegaX8Timer0>
            tcnt_reg,
            ocra_reg,
            ocrb_reg,
            tccra_reg,
            tccrb_reg;
};

#endif
