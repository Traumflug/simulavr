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

#ifndef HWTIMER
#define HWTIMER

#include "hardware.h"
#include "pinatport.h"
#include "rwmem.h"

class HWPrescaler: public Hardware {
	protected:
		unsigned int preScaleValue; //original has 10 Bit

	public:
		HWPrescaler(AvrDevice *core);
		virtual unsigned int CpuCycle() {
			preScaleValue++;
			if (preScaleValue>1023) preScaleValue=0;
			return 0;
		}

		unsigned int GetValue() { return preScaleValue; }
		void Reset(){preScaleValue=0;}
};

class HWIrqSystem;
class HWTimer01Irq;

class HWTimer0: public Hardware {
	protected:
        AvrDevice *core;
		unsigned char tccr;
		unsigned char tcnt;
		HWPrescaler *prescaler;
		//HWIrqSystem *irqSystem;
		HWTimer01Irq *timer01irq;
		PinAtPort pin_t0;
		bool t0_old; 	//last state of external t0 pin

		enum {
			STOP=0,
			CK,
			CK8,
			CK64,
			CK256,
			CK1024,
			T0_FALLING,
			T0_RISING
		} T_prescaler;


		void TimerCompareAfterCount();

	public:
		HWTimer0(AvrDevice *core, HWPrescaler *p,
                 HWTimer01Irq *s, PinAtPort pi,
                 int n=0);

		void Reset() {
			SetTccr(0);
			SetTcnt(0);
			t0_old=0;

		}
		virtual unsigned int CpuCycle();
		void SetTccr(unsigned char val); // { tccr=val; }
		void SetTcnt(unsigned char val) { tcnt=val; }
		unsigned char GetTccr() { return tccr; }
		unsigned char GetTcnt() { return tcnt; }

        IOReg<HWTimer0> tccr_reg, tcnt_reg;
};


class HWTimer1 : public Hardware {
	protected:
        AvrDevice *core;
		HWPrescaler *prescaler;
		//HWIrqSystem *irqSystem;
		HWTimer01Irq *timer01irq;

		unsigned char tccr1a;
		unsigned char tccr1b;

		/* FIXME: According to the datasheets,
		   there is only ONE temporary 16bit
		   register in the AVR architecture. Combine all
		   the various 16bit registers into one! */
		
		unsigned short tcnt1;
		unsigned char tcnt1htemp;

		unsigned short ocr1a;
		unsigned char ocr1ahtemp;

		unsigned short ocr1b;
		unsigned char ocr1bhtemp;

		unsigned short icr1;
		unsigned char icr1htemp;

		//affected external pins
		//bool t1;
		bool t1_old;

		//bool oc1a;
		//bool oc1b;
		//
		PinAtPort pin_t1;
		PinAtPort pin_oc1a;
		PinAtPort pin_oc1b;
		PinAtPort pin_icp;

		bool icp_old;
		
		bool last_ocr1a;
		bool last_ocr1b;
		unsigned char inputCaptureNoiseCnt; //count for 4 cycles if set in ICNC1

		bool cntDir;
		int topValue; // needed for pwm Mode. 

		enum {
			STOP=0,
			CK,
			CK8,
			CK64,
			CK256,
			CK1024,
			T0_FALLING,
			T0_RISING
		} T_prescaler;

		void TimerCompareAfterCount();

	public:
		HWTimer1(AvrDevice *core, HWPrescaler *p, HWTimer01Irq *s,
                 PinAtPort t1, PinAtPort oca, PinAtPort ocb,
                 PinAtPort pin_icp, int n=0);
		void Reset()
		{

			tccr1a=0;
			tccr1b=0;
			tcnt1=0;
			ocr1a=0;
			ocr1b=0;
			icr1=0;

			last_ocr1a=0;
			last_ocr1b=0;
		}
		virtual unsigned int CpuCycle();
		unsigned char GetTccr1a() { return tccr1a;}
		unsigned char GetTccr1b() { return tccr1b;}

		unsigned char GetTcnt1h() { return tcnt1htemp;}
		unsigned char GetTcnt1l() { tcnt1htemp=tcnt1>>8; return tcnt1&0xff;}
		unsigned char GetOcr1ah() { return ocr1a>>8;}
		unsigned char GetOcr1al() { return ocr1a&0xff;}
		unsigned char GetOcr1bh() { return ocr1b>>8;}
		unsigned char GetOcr1bl() { return ocr1b&0xff;}
		unsigned char GetIcr1h() { return icr1htemp;}
		unsigned char GetIcr1l() { icr1htemp=icr1>>8; return icr1&0xff;}

		void SetTccr1a(unsigned char val);
		void SetTccr1b(unsigned char val); // { tccr1b=val;}

		void SetTcnt1h(unsigned char val) { tcnt1htemp=val;}
		void SetTcnt1l(unsigned char val) { tcnt1=val+(tcnt1htemp<<8);}
		void SetOcr1ah(unsigned char val) { ocr1ahtemp=val;}
		void SetOcr1al(unsigned char val) { ocr1a=val+(ocr1ahtemp<<8);}
		void SetOcr1bh(unsigned char val) { ocr1bhtemp=val;}
		void SetOcr1bl(unsigned char val) { ocr1b=val+(ocr1bhtemp<<8);}

        IOReg<HWTimer1>
            tccr1a_reg,
            tccr1b_reg,
            tcnt1h_reg,
            tcnt1l_reg,
            ocr1ah_reg,
            ocr1al_reg,
            ocr1bh_reg,
            ocr1bl_reg,
            icr1h_reg,
            icr1l_reg;
};


#endif
