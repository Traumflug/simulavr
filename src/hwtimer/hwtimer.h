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
#include "prescalermux.h"
#include "timerirq.h"

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
        bool t0_old;    //last state of external t0 pin

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

//////////////////////////////////////////////////////////////////

//! Basic timer unit with 8 Bit counter
class HWTimer8Bit: public Hardware {
    
    protected:
        AvrDevice *core; //!< pointer to device core
        PrescalerMultiplexer* premx; //!< prescaler multiplexer
        IRQLine* timerOverflow; //!< irq line for overflow interrupt
        unsigned long tcnt; //!< THE timercounter
                            // more than 8 bit because of better overflow detection
        unsigned long last_tcnt; //!< timercounter BEFORE counting operation
                                 // used for detection of OC interrupts, because
                                 // OC interupt is fired if counter counts to OCR + 1!
        int cs; //!< select value for prescaler multiplexer
        int updown_counting; //!< count direction control flag, true, if up/down counting
        bool count_down; //!< counter counts down, used for precise pwm modes
        unsigned long limit_bottom; //!< BOTTOM value for up/down counting
        unsigned long limit_top; //!< TOP value for up/down/counting
        
        //! supports the counter operation, handles overflow interrupt
        void CountTimer();
        //! supports the different operation modes after count operation
        virtual void HandleMode() {}
        //! Register access to set counter
        void SetTcnt(unsigned char val);
        //! Register access to read current counter
        unsigned char GetTcnt();
        
    public:
        //! Create a 8Bit Timer/Counter unit, no output compare units
        HWTimer8Bit(AvrDevice *core,
                    PrescalerMultiplexer *p,
                    int unit,
                    IRQLine* tovr);
        //! Perform a reset of this unit
        void Reset();
        
        virtual unsigned int CpuCycle();
        
        IOReg<HWTimer8Bit> tcnt_reg; //!< counter register
};

//! Timer unit with 8 Bit counter and 1 output compare unit
class HWTimer8Bit1OC: public HWTimer8Bit {
    
    protected:
        //! types of waveform generation modes
        enum WGMtype {
          WGM_NORMAL = 0,
          WGM_PCPWM,
          WGM_CTC,
          WGM_FASTPWM
        };
        //! types of compare match output modes
        enum COMtype {
          COM_NOOP = 0,
          COM_TOGGLE,
          COM_CLEAR,
          COM_SET
        };
        
        IRQLine* timerCompare; //!< irq line for compare interrupt
        PinAtPort ocr_out; //!< output pin for output compare unit
        bool ocr_out_state; //!< saved state for ocr output pin
        unsigned char ocr; //!< output compare register
        unsigned char ocr_db; //!< output compare register (double buffer)
        unsigned char tccr; //!< counter control register (only for GetTccr)
        WGMtype wgm; //!< waveform generation mode
        COMtype com; //!< compare match output mode
        
        virtual void HandleMode();
        //! Compare output handling, if compare match occur or on FOCx bit
        void HandleCompareOutput(COMtype com, bool &ocr_out_state, PinAtPort &ocr_out);
        //! Register access to counter control register
        void SetTccr(unsigned char val);
        //! Register access to read counter control register
        unsigned char GetTccr();
        //! Register access to output compare register
        void SetOcr(unsigned char val);
        //! Register access to read output compare register
        unsigned char GetOcr();
        
    public:
        //! Create a 8Bit Timer/Counter unit, 1 output compare unit
        HWTimer8Bit1OC(AvrDevice *core,
                       PrescalerMultiplexer *p,
                       int unit,
                       IRQLine* tovr,
                       IRQLine* tcomp,
                       PinAtPort pout);
        //! Perform a reset of this unit
        void Reset();
        
        IOReg<HWTimer8Bit1OC> tccr_reg; //!< counter control register
        IOReg<HWTimer8Bit1OC> ocr_reg; //!< output compare register
};

#endif
