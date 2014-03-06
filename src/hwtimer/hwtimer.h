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

#ifndef HWTIMER
#define HWTIMER

#include "hardware.h"
#include "pinatport.h"
#include "rwmem.h"
#include "prescalermux.h"
#include "timerirq.h"
#include "traceval.h"
#include "icapturesrc.h"

//! Basic timer unit
/*! Provides basic timer/counter functionality. Counting clock will be taken
  from a prescaler unit. It provides further at max 3 compare values and
  one input capture unit. */
class BasicTimerUnit: public Hardware, public TraceValueRegister {
    
    private:
        int cs; //!< select value for prescaler multiplexer
        TraceValue* counterTrace; //!< TraceValue instance for counter itself
        bool captureInputState; //!< saved state for input capture
        int icapNCcounter; //!< counter for input capture noise canceler
        bool icapNCstate; //!< state for input capture noise canceler
        
    protected:
        //! types of waveform generation modes
        enum WGMtype {
          WGM_NORMAL = 0,
          WGM_PCPWM_8BIT,
          WGM_PCPWM_9BIT,
          WGM_PCPWM_10BIT,
          WGM_CTC_OCRA,
          WGM_FASTPWM_8BIT,
          WGM_FASTPWM_9BIT,
          WGM_FASTPWM_10BIT,
          WGM_PFCPWM_ICR,
          WGM_PFCPWM_OCRA,
          WGM_PCPWM_ICR,
          WGM_PCPWM_OCRA,
          WGM_CTC_ICR,
          WGM_RESERVED,
          WGM_FASTPWM_ICR,
          WGM_FASTPWM_OCRA,
          WGM_tablesize
        };
        //! types of compare match output modes
        enum COMtype {
          COM_NOOP = 0,
          COM_TOGGLE,
          COM_CLEAR,
          COM_SET
        };
        //! event types for timer/counter
        enum CEtype {
            EVT_TOP_REACHED = 0,  //!< TOP reached for one count cycle
            EVT_MAX_REACHED,      //!< an counter overflow occured
            EVT_BOTTOM_REACHED,   //!< BOTTOM reached for one count cycle
            EVT_COMPARE_1,        //!< compare[0] value reached for one count cycle
            EVT_COMPARE_2,        //!< compare[1] value reached for one count cycle
            EVT_COMPARE_3,        //!< compare[2] value reached for one count cycle
        };
        //! indices for OC units
        enum OCRIDXtype {
            OCRIDX_A = 0,    //!< index for OCR unit A
            OCRIDX_B,        //!< index for OCR unit B
            OCRIDX_C,        //!< index for OCR unit C
            OCRIDX_maxUnits  //!< amount of possible OC units
        };
        typedef void (BasicTimerUnit::*wgmfunc_t)(CEtype);
        
        AvrDevice *core; //!< pointer to device core
        PrescalerMultiplexer* premx; //!< prescaler multiplexer
        IRQLine* timerOverflow; //!< irq line for overflow interrupt
        IRQLine* timerCapture; //!< irq line for capture interrupt

        unsigned long vtcnt; //!< THE timercounter
        unsigned long vlast_tcnt; //!< timercounter BEFORE count operation
                                  // used for detection of count events, because
                                  // most events occur at VALUE + 1, means, that VALUE
                                  // have to appear for one count clock cycle!
        int updown_counting; //!< count direction control flag, true, if up/down counting
        bool count_down; //!< counter counts down, used for precise pwm modes
        unsigned long limit_bottom; //!< BOTTOM value for up/down counting
        unsigned long limit_top; //!< TOP value for counting
        unsigned long limit_max; //!< MAX value for counting

        unsigned long icapRegister; //!< Input capture register
        ICaptureSource* icapSource; //!< Input capture source
        bool icapRisingEdge; //!< Input capture on rising edge
        bool icapNoiseCanceler; //!< Noise canceler for input capturing enabled
        
        WGMtype wgm; //!< waveform generation mode
        wgmfunc_t wgmfunc[WGM_tablesize]; //!< waveform generator mode function table
        unsigned long compare[OCRIDX_maxUnits]; //!< compare values for output compare events
        unsigned long compare_dbl[OCRIDX_maxUnits]; //!< double buffer values for compare values
        bool compareEnable[OCRIDX_maxUnits]; //!< enables compare operation
        COMtype com[OCRIDX_maxUnits]; //!< compare match output mode
        IRQLine* timerCompare[OCRIDX_maxUnits]; //!< irq line for compare interrupt
        PinAtPort* compare_output[OCRIDX_maxUnits]; //!< output pins for compare units
        bool compare_output_state[OCRIDX_maxUnits]; //!< status compare output pin
        
        //! Supports the count operation, emits count events to HandleEvent method
        void CountTimer(void);
        //! Supports the input capture function
        virtual void InputCapture(void);
        //! Receives count events
        /*! CountTimer method counts internal counter depending on count mode
          (updown_counting) and generate events, if special count values are reached
          for at least one counting cycle. It can happen, that more than one event
          could occur in the same count cycle! */
        void HandleEvent(CEtype event) { (this->*wgmfunc[wgm])(event); }
        //! Set clock mode
        void SetClockMode(int _cs);
        //! Set the counter itself
        void SetCounter(unsigned long val);
        //! Set compare output mode
        void SetCompareOutputMode(int idx, COMtype mode);
        //! Set compare output pins in non pwm mode
        void SetCompareOutput(int idx);
        //! Set compare output pins in pwm mode
        void SetPWMCompareOutput(int idx, bool topOrDown);
        
        //! returns true, if WGM is in one of the PWM modes
        bool WGMisPWM(void) { return wgm != WGM_NORMAL && wgm != WGM_CTC_OCRA && wgm != WGM_CTC_ICR; }
        //! returns true, if WGM uses IC register for defining TOP counter value
        bool WGMuseICR(void) { return wgm == WGM_FASTPWM_ICR || wgm == WGM_CTC_ICR || wgm == WGM_PCPWM_ICR || wgm == WGM_PFCPWM_ICR; }
        //! WGM noop function
        void WGMFunc_noop(CEtype event) {}
        //! WGM function for normal mode (unique for all different timers)
        void WGMfunc_normal(CEtype event);
        //! WGM function for ctc mode (unique for all different timers)
        void WGMfunc_ctc(CEtype event);
        //! WGM function for fast pwm mode (unique for all different timers)
        void WGMfunc_fastpwm(CEtype event);
        //! WGM function for phase correct pwm mode (unique for all different timers)
        void WGMfunc_pcpwm(CEtype event);
        //! WGM function for phase and frequency correct pwm mode (unique for all different timers)
        void WGMfunc_pfcpwm(CEtype event);
        
    public:
        //! Create a basic Timer/Counter unit
        BasicTimerUnit(AvrDevice *core,
                       PrescalerMultiplexer *p,
                       int unit,
                       IRQLine* tov,
                       IRQLine* tcap,
                       ICaptureSource* icapsrc,
                       int countersize = 8);
        ~BasicTimerUnit();
        //! Perform a reset of this unit
        void Reset();
        
        //! Process timer/counter unit operations by CPU cycle
        virtual unsigned int CpuCycle();

        //! register analog comparator unit for input capture source
        void RegisterACompForICapture(HWAcomp *acomp);

        //! reflect ACIC flag to input capture source
        void SetACIC(bool acic) { if(icapSource != NULL) icapSource->SetACIC(acic); }
};

//! Extends BasicTimerUnit to provide common support to all types of 8Bit timer units
class HWTimer8: public BasicTimerUnit {
    
    protected:
        //! Change WGM mode, set counter limits
        void ChangeWGM(WGMtype mode);
        
        //! Setter method for compare register
        void SetCompareRegister(int idx, unsigned char val);
        //! Getter method for compare register
        unsigned char GetCompareRegister(int idx);
        
        //! Register access to set counter register high byte
        void Set_TCNT(unsigned char val) { SetCounter(val); }
        //! Register access to read counter register high byte
        unsigned char Get_TCNT() { return vtcnt & 0xff; }

        //! Register access to set output compare register A
        void Set_OCRA(unsigned char val) { SetCompareRegister(0, val); }
        //! Register access to read output compare register A
        unsigned char Get_OCRA() { return GetCompareRegister(0); }
        
        //! Register access to set output compare register B
        void Set_OCRB(unsigned char val) { SetCompareRegister(1, val); }
        //! Register access to read output compare register B
        unsigned char Get_OCRB() { return GetCompareRegister(1); }
        
    public:
        IOReg<HWTimer8> tcnt_reg; //!< counter register
        IOReg<HWTimer8> ocra_reg; //!< output compare A register
        IOReg<HWTimer8> ocrb_reg; //!< output compare B register
        
        HWTimer8(AvrDevice *core,
                 PrescalerMultiplexer *p,
                 int unit,
                 IRQLine* tov,
                 IRQLine* tcompA,
                 PinAtPort* outA,
                 IRQLine* tcompB,
                 PinAtPort* outB);
        //! Perform a reset of this unit
        void Reset();
};

//! Extends BasicTimerUnit to provide common support to all types of 16Bit timer units
class HWTimer16: public BasicTimerUnit {
    
    protected:
        //! the high byte temporary register for read/write access to TCNT and ICR
        unsigned char accessTempRegister;
        
        //! Setter method for compare register
        void SetCompareRegister(int idx, bool high, unsigned char val);
        //! Getter method for compare register
        unsigned char GetCompareRegister(int idx, bool high);
        //! Setter method for TCNT and ICR register
        void SetComplexRegister(bool is_icr, bool high, unsigned char val);
        //! Getter method for TCNT and ICR register
        unsigned char GetComplexRegister(bool is_icr, bool high);
        
        //! Change WGM mode, set counter limits
        void ChangeWGM(WGMtype mode);
        
        //! Register access to set counter register high byte
        void Set_TCNTH(unsigned char val) { SetComplexRegister(false, true, val); }
        //! Register access to read counter register high byte
        unsigned char Get_TCNTH() { return GetComplexRegister(false, true); }
        //! Register access to set counter register low byte
        void Set_TCNTL(unsigned char val) { SetComplexRegister(false, false, val); }
        //! Register access to read counter register low byte
        unsigned char Get_TCNTL() { return GetComplexRegister(false, false); }

        //! Register access to set output compare register A high byte
        void Set_OCRAH(unsigned char val) { SetCompareRegister(0, true, val); }
        //! Register access to read output compare register A high byte
        unsigned char Get_OCRAH() { return GetCompareRegister(0, true); }
        //! Register access to set output compare register A low byte
        void Set_OCRAL(unsigned char val) { SetCompareRegister(0, false, val); }
        //! Register access to read output compare register A low byte
        unsigned char Get_OCRAL() { return GetCompareRegister(0, false); }
        
        //! Register access to set output compare register B high byte
        void Set_OCRBH(unsigned char val) { SetCompareRegister(1, true, val); }
        //! Register access to read output compare register B high byte
        unsigned char Get_OCRBH() { return GetCompareRegister(1, true); }
        //! Register access to set output compare register B low byte
        void Set_OCRBL(unsigned char val) { SetCompareRegister(1, false, val); }
        //! Register access to read output compare register B low byte
        unsigned char Get_OCRBL() { return GetCompareRegister(1, false); }
        
        //! Register access to set output compare register C high byte
        void Set_OCRCH(unsigned char val) { SetCompareRegister(2, true, val); }
        //! Register access to read output compare register C high byte
        unsigned char Get_OCRCH() { return GetCompareRegister(2, true); }
        //! Register access to set output compare register C low byte
        void Set_OCRCL(unsigned char val) { SetCompareRegister(2, false, val); }
        //! Register access to read output compare register C low byte
        unsigned char Get_OCRCL() { return GetCompareRegister(2, false); }
        
        //! Register access to set input capture register high byte
        void Set_ICRH(unsigned char val) { SetComplexRegister(true, true, val); }
        //! Register access to read input capture register high byte
        unsigned char Get_ICRH() { return GetComplexRegister(true, true); }
        //! Register access to set input capture register low byte
        void Set_ICRL(unsigned char val) { SetComplexRegister(true, false, val); }
        //! Register access to read input capture register low byte
        unsigned char Get_ICRL() { return GetComplexRegister(true, false); }
        
    public:
        IOReg<HWTimer16> tcnt_h_reg; //!< counter register, high byte
        IOReg<HWTimer16> tcnt_l_reg; //!< counter register, low byte
        IOReg<HWTimer16> ocra_h_reg; //!< output compare A register, high byte
        IOReg<HWTimer16> ocra_l_reg; //!< output compare A register, low byte
        IOReg<HWTimer16> ocrb_h_reg; //!< output compare B register, high byte
        IOReg<HWTimer16> ocrb_l_reg; //!< output compare B register, low byte
        IOReg<HWTimer16> ocrc_h_reg; //!< output compare C register, high byte
        IOReg<HWTimer16> ocrc_l_reg; //!< output compare C register, low byte
        IOReg<HWTimer16> icr_h_reg; //!< input capture register, high byte
        IOReg<HWTimer16> icr_l_reg; //!< input capture register, low byte
        
        HWTimer16(AvrDevice *core,
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
                  ICaptureSource* icapsrc);
        //! Perform a reset of this unit
        void Reset(void);
};

//! Timer unit with 8Bit counter and no output compare unit
/*! This timer unit is used by following devices: ATMega8.
  It has only 1 mode: normal counting!

  TCCRx register contains the following configuration bits (x=#timer):
  
  \verbatim
  +---+---+---+---+---+----+----+----+
  | - | - | - | - | - |CSx2|CSx1|CSx0|
  +---+---+---+---+---+----+----+----+ \endverbatim */
class HWTimer8_0C: public HWTimer8 {
    
    protected:
        unsigned char tccr_val; //!< register value TCCR
        
        //! Register access to set control register
        void Set_TCCR(unsigned char val);
        //! Register access to read control register
        unsigned char Get_TCCR() { return tccr_val; }
        
    public:
        IOReg<HWTimer8_0C> tccr_reg; //!< control register
        
        HWTimer8_0C(AvrDevice *core,
                    PrescalerMultiplexer *p,
                    int unit,
                    IRQLine* tov);
        //! Perform a reset of this unit
        void Reset(void);
};

//! Timer unit with 8Bit counter and one output compare unit
/*! This timer unit is used by following devices: ATMega128.

  TCCRx register contains the following configuration bits (x=#timer):
  
  \verbatim
  +----+-----+-----+-----+-----+----+----+----+
  |FOCx|WGMx0|COMx1|COMx0|WGMx1|CSx2|CSx1|CSx0|
  +----+-----+-----+-----+-----+----+----+----+ \endverbatim */
class HWTimer8_1C: public HWTimer8 {
    
    protected:
        unsigned char tccr_val; //!< register value TCCR
        
        //! Register access to set control register
        void Set_TCCR(unsigned char val);
        //! Register access to read control register
        unsigned char Get_TCCR() { return tccr_val; }
        
    public:
        IOReg<HWTimer8_1C> tccr_reg; //!< control register
        
        HWTimer8_1C(AvrDevice *core,
                    PrescalerMultiplexer *p,
                    int unit,
                    IRQLine* tov,
                    IRQLine* tcompA,
                    PinAtPort* outA);
        //! Perform a reset of this unit
        void Reset(void);
};

//! Timer unit with 8Bit counter and 2 output compare unit
/*! This timer unit is used by following devices: ATMega48/88/168/328.

  TCCRxA register contains the following configuration bits (x=#timer):
  
  \verbatim
  +------+------+------+------+---+---+-----+-----+
  |COMxA1|COMxA0|COMxB1|COMxB0| - | - |WGMx1|WGMx0|
  +------+------+------+------+---+---+-----+-----+ \endverbatim
  
  TCCRxB register contains the following configuration bits (x=#timer):
  
  \verbatim
  +-----+-----+---+---+-----+----+----+----+
  |FOCxA|FOCxB| - | - |WGMx2|CSx2|CSx1|CSx0|
  +-----+-----+---+---+-----+----+----+----+ \endverbatim */
class HWTimer8_2C: public HWTimer8 {
    
    private:
        int wgm_raw; //!< this is the wgm raw value from register
        
        //! Handle special WGM setting, translate wgm raw value to wgm value
        void Set_WGM(int val);
    
    protected:
        unsigned char tccra_val; //!< register value TCCRA
        unsigned char tccrb_val; //!< register value TCCRB
        
        //! Register access to set control register
        void Set_TCCRA(unsigned char val);
        //! Register access to read control register
        unsigned char Get_TCCRA() { return tccra_val; }
        
        //! Register access to set control register
        void Set_TCCRB(unsigned char val);
        //! Register access to read control register
        unsigned char Get_TCCRB() { return tccrb_val; }
        
    public:
        IOReg<HWTimer8_2C> tccra_reg; //!< control register A
        IOReg<HWTimer8_2C> tccrb_reg; //!< control register B
        
        HWTimer8_2C(AvrDevice *core,
                    PrescalerMultiplexer *p,
                    int unit,
                    IRQLine* tov,
                    IRQLine* tcompA,
                    PinAtPort* outA,
                    IRQLine* tcompB,
                    PinAtPort* outB);
        //! Perform a reset of this unit
        void Reset(void);
};

//! Timer unit with 16Bit counter and one output compare unit
/*! This timer unit is used by following devices: AT90[L]S4433.

  TCCRxA register contains the following configuration bits (x=#timer):
  
  \verbatim
  +-----+-----+---+---+---+---+-----+-----+
  |COMx1|COMx0| - | - | - | - |PWMx1|PWMx0|
  +-----+-----+---+---+---+---+-----+-----+ \endverbatim
  
  TCCRxB register contains the following configuration bits (x=#timer):
  
  \verbatim
  +-----+-----+---+---+----+----+----+----+
  |ICNCx|ICESx| - | - |CTCx|CSx2|CSx1|CSx0|
  +-----+-----+---+---+----+----+----+----+ \endverbatim */
class HWTimer16_1C: public HWTimer16 {
    
    private:
        int wgm_raw; //!< this is the wgm raw value from register
        
        //! Handle special WGM setting, translate wgm raw value to wgm value
        void Set_WGM(int val);
        
    protected:
        unsigned char tccra_val; //!< register value TCCRA
        unsigned char tccrb_val; //!< register value TCCRB
        
        //! Register access to set control register A
        void Set_TCCRA(unsigned char val);
        //! Register access to read control register A
        unsigned char Get_TCCRA() { return tccra_val; }
        
        //! Register access to set control register B
        void Set_TCCRB(unsigned char val);
        //! Register access to read control register B
        unsigned char Get_TCCRB() { return tccrb_val; }
        
    public:
        IOReg<HWTimer16_1C> tccra_reg; //!< control register A
        IOReg<HWTimer16_1C> tccrb_reg; //!< control register B
        
        HWTimer16_1C(AvrDevice *core,
                     PrescalerMultiplexer *p,
                     int unit,
                     IRQLine* tov,
                     IRQLine* tcompA,
                     PinAtPort* outA,
                     IRQLine* ticap,
                     ICaptureSource* icapsrc);
        //! Perform a reset of this unit
        void Reset(void);
};

//! Timer unit with 16Bit counter and 2 output compare units and 2 config registers
/*! This timer unit is used by following devices: ATMega16, AT90S8515.

  TCCRxA register contains the following configuration bits (x=#timer):
  
  \verbatim
  +------+------+------+------+-----+-----+-----+-----+
  |COMxA1|COMxA0|COMxB1|COMxB0|FOCxA|FOCxB|WGMx1|WGMx0|
  +------+------+------+------+-----+-----+-----+-----+ \endverbatim
  
  On AT90S8515 FOCx bits are not available, WGMxy bits are named PWMxy!
  
  TCCRxB register contains the following configuration bits (x=#timer):
  
  \verbatim
  +-----+-----+---+-----+-----+----+----+----+
  |ICNCx|ICESx| - |WGMx3|WGMx2|CSx2|CSx1|CSx0|
  +-----+-----+---+-----+-----+----+----+----+ \endverbatim
  
  On AT90S8515 WGMx3 bit is not available, WGMx2 bit is named CTCx! */
class HWTimer16_2C2: public HWTimer16 {
    
    private:
        int wgm_raw; //!< this is the wgm raw value from register
        bool at8515_mode; //!< signals, that this timer units is used in AT90S8515
        
        //! Handle special WGM setting, translate wgm raw value to wgm value
        void Set_WGM(int val);
    
    protected:
        unsigned char tccra_val; //!< register value TCCRA
        unsigned char tccrb_val; //!< register value TCCRB
        
        //! Register access to set control register A
        void Set_TCCRA(unsigned char val);
        //! Register access to read control register A
        unsigned char Get_TCCRA() { return tccra_val; }
        
        //! Register access to set control register B
        void Set_TCCRB(unsigned char val);
        //! Register access to read control register B
        unsigned char Get_TCCRB() { return tccrb_val; }
        
    public:
        IOReg<HWTimer16_2C2> tccra_reg; //!< control register A
        IOReg<HWTimer16_2C2> tccrb_reg; //!< control register B
        
        HWTimer16_2C2(AvrDevice *core,
                      PrescalerMultiplexer *p,
                      int unit,
                      IRQLine* tov,
                      IRQLine* tcompA,
                      PinAtPort* outA,
                      IRQLine* tcompB,
                      PinAtPort* outB,
                      IRQLine* ticap,
                      ICaptureSource* icapsrc,
                      bool is_at8515);
        //! Perform a reset of this unit
        void Reset(void);
};

//! Timer unit with 16Bit counter and 2 output compare units, but 3 config registers
/*! This timer unit is used by following devices: ATMega48/88/168/328.

  TCCRxA register contains the following configuration bits (x=#timer):
  
  \verbatim
  +------+------+------+------+---+---+-----+-----+
  |COMxA1|COMxA0|COMxB1|COMxB0| - | - |WGMx1|WGMx0|
  +------+------+------+------+---+---+-----+-----+ \endverbatim
  
  TCCRxB register contains the following configuration bits (x=#timer):
  
  \verbatim
  +----+------+---+-----+-----+----+----+----+
  |ICNCx|ICESx| - |WGMx3|WGMx2|CSx2|CSx1|CSx0|
  +----+------+---+-----+-----+----+----+----+ \endverbatim
  
  TCCRxC register contains the following configuration bits (x=#timer):
  
  \verbatim
  +-----+-----+---+---+---+---+---+---+ 
  |FOCxA|FOCxB| - | - | - | - | - | - |
  +-----+-----+---+---+---+---+---+---+ \endverbatim */
class HWTimer16_2C3: public HWTimer16 {
    
    protected:
        unsigned char tccra_val; //!< register value TCCRA
        unsigned char tccrb_val; //!< register value TCCRB
        
        //! Register access to set control register A
        void Set_TCCRA(unsigned char val);
        //! Register access to read control register A
        unsigned char Get_TCCRA() { return tccra_val; }
        
        //! Register access to set control register B
        void Set_TCCRB(unsigned char val);
        //! Register access to read control register B
        unsigned char Get_TCCRB() { return tccrb_val; }
        
        //! Register access to set control register C
        void Set_TCCRC(unsigned char val);
        //! Register access to read control register C
        unsigned char Get_TCCRC() { return 0; } // will be read allways 0!
        
    public:
        IOReg<HWTimer16_2C3> tccra_reg; //!< control register A
        IOReg<HWTimer16_2C3> tccrb_reg; //!< control register B
        IOReg<HWTimer16_2C3> tccrc_reg; //!< control register C
        
        HWTimer16_2C3(AvrDevice *core,
                      PrescalerMultiplexer *p,
                      int unit,
                      IRQLine* tov,
                      IRQLine* tcompA,
                      PinAtPort* outA,
                      IRQLine* tcompB,
                      PinAtPort* outB,
                      IRQLine* ticap,
                      ICaptureSource* icapsrc);
        //! Perform a reset of this unit
        void Reset(void);
};

//! Timer unit with 16Bit counter and 3 output compare units
/*! This timer unit is used by following devices: ATMega128.

  TCCRxA register contains the following configuration bits (x=#timer):
  
  \verbatim
  +------+------+------+------+------+------+-----+-----+
  |COMxA1|COMxA0|COMxB1|COMxB0|COMxC1|COMxC0|WGMx1|WGMx0|
  +------+------+------+------+------+------+-----+-----+ \endverbatim
  
  TCCRxB register contains the following configuration bits (x=#timer):
  
  \verbatim
  +----+------+---+-----+-----+----+----+----+
  |ICNCx|ICESx| - |WGMx3|WGMx2|CSx2|CSx1|CSx0|
  +----+------+---+-----+-----+----+----+----+ \endverbatim
  
  TCCRxC register contains the following configuration bits (x=#timer):
  
  \verbatim
  +-----+-----+-----+---+---+---+---+---+ 
  |FOCxA|FOCxB|FOCxC| - | - | - | - | - |
  +-----+-----+-----+---+---+---+---+---+ \endverbatim */
class HWTimer16_3C: public HWTimer16 {
    
    protected:
        unsigned char tccra_val; //!< register value TCCRA
        unsigned char tccrb_val; //!< register value TCCRB
        
        //! Register access to set control register A
        void Set_TCCRA(unsigned char val);
        //! Register access to read control register A
        unsigned char Get_TCCRA() { return tccra_val; }
        
        //! Register access to set control register B
        void Set_TCCRB(unsigned char val);
        //! Register access to read control register B
        unsigned char Get_TCCRB() { return tccrb_val; }
        
        //! Register access to set control register C
        void Set_TCCRC(unsigned char val);
        //! Register access to read control register C
        unsigned char Get_TCCRC() { return 0; } // will be read allways 0!
        
    public:
        IOReg<HWTimer16_3C> tccra_reg; //!< control register A
        IOReg<HWTimer16_3C> tccrb_reg; //!< control register B
        IOReg<HWTimer16_3C> tccrc_reg; //!< control register C
        
        HWTimer16_3C(AvrDevice *core,
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
                     ICaptureSource* icapsrc);
        //! Perform a reset of this unit
        void Reset(void);
};

//! PWM output unit for timer 1 on ATtiny25/45/85
/*! Supports the different output control modes for a OCR unit on timer 1 in
 *  ATtiny25/45/85 devices, contains also implementation of dead time generator
 */
class TimerTinyX5_OCR {
    private:
        PinAtPort* outPin;           //!< normal output pin for OCR unit
        PinAtPort* outPinInv;        //!< inverted output pin for OCR unit

        int ocrComMode;              //!< COM mode
        bool ocrPWM;                 //!< flag, if OCR unit is in PWM mode
        bool ocrOut;                 //!< OCR status before dead time generator
        int dtHigh;                  //!< dead time raise delay
        int dtLow;                   //!< dead time fall delay
        int dtCounter;               //!< dead time counter

        //! Calculate output pin value (before dead time generator)
        void SetPWM(bool isCompareEvent);

        //! Calculate output pin value after dead time generator
        void SetDeadTime(bool pwmValue);

    public:
        TimerTinyX5_OCR(PinAtPort* pinOut, PinAtPort* pinOutInv);

        //! Reset internal states on device reset
        void Reset();

        //! Run one clock cycle from dead time prescaler
        void DTClockCycle();

        //! OCR event
        void TimerEvent(bool isCompareEvent) { SetPWM(isCompareEvent); }

        //! Manual change of OCR unit by force bit
        void ForceEvent() { /* only, if unit isn't in PWM mode */ if(!ocrPWM) SetPWM(true); }

        //! Configure dead time counter
        void SetDeadTime(int highTime, int lowTime) { dtHigh = highTime; dtLow = lowTime; }

        //! Configure OCR mode
        void SetOCRMode(bool isPWM, int comMode);
};

//! Helper class to simulate transfer of register values from bus area to timer async area
/*! This isn't a exact simulation, because it delays the register settings only for one
  clock cycle. As shown on datasheet it's 1 1/2 clock in sync mode and 1 to 2 clocks in
  async mode! */
class HWTimerTinyX5_SyncReg {
    private:
        unsigned char inValue; //!< input register value
        unsigned char regValue; //!< valid register value inside sync area

    public:
        HWTimerTinyX5_SyncReg() { Reset(0); }

        //! perform a reset to set valid reset values without clock
        void Reset(unsigned char v) { inValue = regValue = v; }

        //! assign new register value
        unsigned char operator=(unsigned char v) { inValue = v; return v; }

        //! read register value inside sync area
        operator unsigned char() { return regValue; }

        //! read register value on input area
        unsigned char GetBusValue(void) { return inValue; }

        //! check after one clock, if register value has changed
        bool ClockAndChanged(void) { if(inValue != regValue) { regValue = inValue; return true; } return false; }

        //! Mask out a value inside sync area and do not force a change event
        void MaskOutSync(unsigned char mask) { inValue &= ~mask; regValue = inValue; }
};

//! timer unit for timer 1 on ATtiny25/45/85
/*! Timer1 on ATtiny25/45/85 is an async timer, which can be clocked till 64MHz by pll
  from system clock. */
class HWTimerTinyX5: public Hardware,
    public TraceValueRegister,
    public SimulationMember,
    public IOSpecialRegClient {

    private:
        TraceValue* counterTrace;     //!< TraceValue instance for counter itself
        TraceValue* prescalerTrace;   //!< TraceValue instance for prescaler
        TraceValue* dTPrescalerTrace; //!< TraceValue instance for dead time prescaler

        // counter and prescaler
        unsigned long counter;     //!< THE timer counter
        unsigned long prescaler;   //!< THE prescaler counter
        unsigned char dtprescaler; //!< dead time prescaler counter

        // input/output values for TCCR, OCRx and input value for GTCCR
        HWTimerTinyX5_SyncReg tccr_inout_val;  //!< register value TCCR1
        HWTimerTinyX5_SyncReg ocra_inout_val;  //!< register value OCRA
        HWTimerTinyX5_SyncReg ocrb_inout_val;  //!< register value OCRB
        HWTimerTinyX5_SyncReg ocrc_inout_val;  //!< register value OCRC
        HWTimerTinyX5_SyncReg gtccr_in_val;    //!< input register value GTCCR
        unsigned char dtps1_inout_val;         //!< register value DTPS1
        HWTimerTinyX5_SyncReg dt1a_inout_val;  //!< register value DT1A
        HWTimerTinyX5_SyncReg dt1b_inout_val;  //!< register value DT1B

        // output and input register for TCNT
        unsigned char tcnt_out_val;       //!< output register value for TCNT
        unsigned char tcnt_out_async_tmp; //!< temporary register value for TCNT in async mode
        unsigned char tcnt_in_val;        //!< input register value for TCNT
        bool tcnt_set_flag;               //!< flag to signal, that a new counter value was set
        bool tov_internal_flag;           //!< TOV flag is set, have to be delayed by 1 CK
        bool tocra_internal_flag;         //!< OCFxA flag is set, have to be delayed by 1 CK
        bool tocrb_internal_flag;         //!< OCFxB flag is set, have to be delayed by 1 CK

        // internal values for TCCR, OCRx
        unsigned char ocra_internal_val;  //!< internal (async) register value for OCRA1
        unsigned long ocra_compare;       //!< active compare value for OCR A unit
        TimerTinyX5_OCR ocra_unit;        //!< OCR control unit for OCR channel A
        unsigned char ocrb_internal_val;  //!< internal (async) register value for OCRB1
        unsigned long ocrb_compare;       //!< active compare value for OCR B unit
        TimerTinyX5_OCR ocrb_unit;        //!< OCR control unit for OCR channel B
        int cfg_prescaler;                //!< internal (async) prescaler setting
        int cfg_dtprescaler;              //!< internal (async) dead time prescaler setting
        int cfg_mode;                     //!< internal (async) timer mode setting
        bool cfg_ctc;                     //!< internal (async) flag for clear timer counter
        int cfg_com_a;                    //!< internal (async) setting for compare output modul A
        int cfg_com_b;                    //!< internal (async) setting for compare output modul B

        enum TMODEtype {
            TMODE_NORMAL = 0x0, //!< timer in normal mode, upcounting from 0x0 to 0xff or 0 to OCRC (CTC mode)
            TMODE_PWMA = 0x1,   //!< timer in PWM mode, upcounting from 0 to OCRC, PWM A active
            TMODE_PWMB = 0x2    //!< timer in PWM mode, upcounting from 0 to OCRC, PWM B active
        };

        // variables for async mode and pll
        int  asyncClock_step;                  //!< step counter for step delays. -1, if not in async mode
        bool asyncClock_async;                 //!< mode switch for async mode
        bool asyncClock_lsm;                   //!< mode switch for lsm mode (32MHz clock)
        bool asyncClock_pll;                   //!< pll is switched on
        bool asyncClock_plllock;               //!< pll frequency is locked
        SystemClockOffset asyncClock_locktime; //!< time, when pll is locked

    protected:
        AvrDevice *core;              //!< pointer to device core
        IOSpecialReg* gtccrRegister;  //!< instance of GTCCR register
        IOSpecialReg* pllcsrRegister; //!< instance of PLLCSR register
        IOSpecialReg* dtps1Register;  //!< instance of DTPS1 register
        IOSpecialReg* dt1aRegister;   //!< instance of DT1A register
        IOSpecialReg* dt1bRegister;   //!< instance of DT1B register
        IRQLine* timerOverflowInt;    //!< irq line for overflow interrupt
        IRQLine* timerOCRAInt;        //!< irq line for output compare A interrupt
        IRQLine* timerOCRBInt;        //!< irq line for output compare B interrupt

        //! Register access to set counter register
        void Set_TCNT(unsigned char val) { tcnt_in_val = val; tcnt_set_flag = true; }
        //! Register access to read counter register
        unsigned char Get_TCNT() { return tcnt_out_val; }

        //! Register access to set control register
        void Set_TCCR(unsigned char val) { tccr_inout_val = val; }
        //! Register access to read control register
        unsigned char Get_TCCR() { return tccr_inout_val.GetBusValue(); }

        //! Register access to set output compare register A
        void Set_OCRA(unsigned char val) { ocra_inout_val = val; }
        //! Register access to read output compare register A
        unsigned char Get_OCRA() { return ocra_inout_val.GetBusValue(); }

        //! Register access to set output compare register B
        void Set_OCRB(unsigned char val) { ocrb_inout_val = val; }
        //! Register access to read output compare register B
        unsigned char Get_OCRB() { return ocrb_inout_val.GetBusValue(); }

        //! Register access to set output compare register C
        void Set_OCRC(unsigned char val) { ocrc_inout_val = val; }
        //! Register access to read output compare register C
        unsigned char Get_OCRC() { return ocrc_inout_val.GetBusValue(); }

        //! Register access to set dead time prescaler
        void Set_DTPS1(unsigned char val) { dtps1_inout_val = val; }
        //! Register access to read dead time prescaler
        unsigned char Get_DTPS1() { return dtps1_inout_val; }

        //! Register access to set dead time value for channel A
        void Set_DT1A(unsigned char val) { dt1a_inout_val = val; }
        //! Register access to read dead time value for channel A
        unsigned char Get_DT1A() { return dt1a_inout_val.GetBusValue(); }

        //! Register access to set dead time value for channel B
        void Set_DT1B(unsigned char val) { dt1b_inout_val = val; }
        //! Register access to read dead time value for channel B
        unsigned char Get_DT1B() { return dt1b_inout_val.GetBusValue(); }

        //! IO register interface set method, see IOSpecialRegClient
        unsigned char set_from_reg(const IOSpecialReg *reg, unsigned char nv);
        //! IO register interface get method, see IOSpecialRegClient
        unsigned char get_from_client(const IOSpecialReg *reg, unsigned char v);

        //! Set clock source for prescaler
        void SetPrescalerClock(bool pcke);

        //! Count function, contains prescaler, multiplexer and counter functionality
        void TimerCounter(void);

        //! Prescaler multiplex function, returns true, if a count pulse is happen
        bool PrescalerMux(void);

        //! Dead time prescaler multiplex function, returns true, if a count pulse is happen
        bool DeadTimePrescalerMux(void);

        //! Transfer register input to internal register set
        void TransferInputValues(void);

        //! Transfer internal register values (if needed) to by core readable register
        void TransferOutputValues(void);

    public:
        IOReg<HWTimerTinyX5> tccr_reg;  //!< control register
        IOReg<HWTimerTinyX5> tcnt_reg;  //!< counter register
        IOReg<HWTimerTinyX5> tocra_reg; //!< OCR register channel A
        IOReg<HWTimerTinyX5> tocrb_reg; //!< OCR register channel B
        IOReg<HWTimerTinyX5> tocrc_reg; //!< OCR register channel C
        IOReg<HWTimerTinyX5> dtps1_reg; //!< dead time generator prescaler register
        IOReg<HWTimerTinyX5> dt1a_reg;  //!< dead time generator register channel A
        IOReg<HWTimerTinyX5> dt1b_reg;  //!< dead time generator register channel B

        HWTimerTinyX5(AvrDevice *core,
                      IOSpecialReg *gtccr,
                      IOSpecialReg *pllcsr,
                      IRQLine* tov,
                      IRQLine* tocra,
                      PinAtPort* ocra_out,
                      PinAtPort* ocra_outinv,
                      IRQLine* tocrb,
                      PinAtPort* ocrb_out,
                      PinAtPort* ocrb_outinv);
        ~HWTimerTinyX5();

        //! Performs the async clocking, if necessary
        int Step(bool &untilCoreStepFinished, SystemClockOffset *nextStepIn_ns);
        //! Perform a reset of this unit
        void Reset();
        //! Process timer/counter unit operations by CPU cycle
        unsigned int CpuCycle();
};

#endif
