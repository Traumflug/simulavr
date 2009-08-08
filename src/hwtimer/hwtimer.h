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

#endif
