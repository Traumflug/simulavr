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

#ifndef HWAD
#define HWAD

#include "hardware.h"
#include "avrdevice.h"
#include "rwmem.h"
#include "traceval.h"

//! Reference source for ADC (base class)
class HWARef {

    protected:
        AvrDevice *core;

    public:
        HWARef(AvrDevice *_core): core(_core) { }
        virtual ~HWARef() { }

        virtual float GetRefValue(int select, float vcc) = 0;
};

//! ADC reference is taken from special ADREF pin (no port pin)
class HWARefPin: public HWARef {

    protected:
        Pin aref_pin;

    public:
        HWARefPin(AvrDevice *_core);

        virtual float GetRefValue(int select, float vcc);
};

//! ADC reference is selected on 3 or 4 different sources: Vcc, aref pin, bandgap or 2.56V reference
class HWARef4: public HWARefPin {

    protected:
        int refType;

    public:
        enum {
            REFTYPE_NOBG, //!< 0:aref, 1:vcc, 2:-,  3:2.56V
            REFTYPE_BG3,  //!< 0:aref, 1:vcc, 2:bg, 3:2.56V
            REFTYPE_BG4   //!< 0:aref, 1:vcc, 2:-,  3:bg
        };

        HWARef4(AvrDevice *_core, int _type);

        virtual float GetRefValue(int select, float vcc);
};

//! ADC reference is selected on 4 diff. sources: Vcc, aref pin, bandgap or 2.56V reference
class HWARef8: public HWARef {

    protected:
        Pin* aref_pin;

    public:
        HWARef8(AvrDevice *_core, Pin* _refpin): HWARef(_core), aref_pin(_refpin) { }

        virtual float GetRefValue(int select, float vcc);
};

/** ADC multiplexer base class */
class HWAdmux: public HasPinNotifyFunction {

    protected:
        Pin* ad[8]; // 4 to 8 pins selectable from the mux
        AnalogSignalChange *notifyClient;
        int muxSelect; //! Multiplexer channel, can't be used for ADC sampling because of buffering on conversion start!
        int numPins;
        AvrDevice *core;

    public:
        HWAdmux(AvrDevice* _core, int _pins):
                                              notifyClient(0), 
                                              muxSelect(0),
                                              numPins(_pins),
            core(_core)
    { }
        virtual ~HWAdmux() { }

        virtual float GetValue(int select, float vcc) = 0;
        virtual float GetValueAComp(int select, float vcc) { return 0.0; }
        virtual bool IsDifferenceChannel(int select) { return false; }
        void SetMuxSelect(int select);
        void PinStateHasChanged(Pin*);
        void RegisterNotifyClient(AnalogSignalChange *client) { notifyClient = client; }
        void UnregisterNotifyClient(void) { notifyClient = 0; }
};

class HWAdmux6: public HWAdmux {

    public:
        HWAdmux6(AvrDevice* c, Pin*  _ad0,
                               Pin*  _ad1,
                               Pin*  _ad2,
                               Pin*  _ad3,
                               Pin*  _ad4,
                               Pin*  _ad5);

        virtual float GetValue(int select, float vcc);
};

class HWAdmuxM8: public HWAdmux {

    protected:
        HWAdmuxM8(AvrDevice* c, Pin*  _ad0,
                                Pin*  _ad1,
                                Pin*  _ad2,
                                Pin*  _ad3);

    public:
        HWAdmuxM8(AvrDevice* c, Pin*  _ad0,
                                Pin*  _ad1,
                                Pin*  _ad2,
                                Pin*  _ad3,
                                Pin*  _ad4,
                                Pin*  _ad5,
                                Pin*  _ad6,
                                Pin*  _ad7);
        virtual float GetValue(int select, float vcc);
        virtual float GetValueAComp(int select, float vcc);
};

class HWAdmuxM16: public HWAdmuxM8 {

    public:
        HWAdmuxM16(AvrDevice* c, Pin*  _ad0,
                                 Pin*  _ad1,
                                 Pin*  _ad2,
                                 Pin*  _ad3,
                                 Pin*  _ad4,
                                 Pin*  _ad5,
                                 Pin*  _ad6,
                                 Pin*  _ad7);

        virtual float GetValue(int select, float vcc);
        virtual bool IsDifferenceChannel(int select);
};

class HWAdmuxT25: public HWAdmuxM8 {

    public:
        HWAdmuxT25(AvrDevice* c, Pin*  _ad0,
                                 Pin*  _ad1,
                                 Pin*  _ad2,
                                 Pin*  _ad3);

        virtual float GetValue(int select, float vcc);
        virtual bool IsDifferenceChannel(int select);
};

/** Analog-digital converter (ADC) */
class HWAd: public Hardware, public TraceValueRegister, public AnalogSignalChange {

    protected:
        int adType;
        unsigned char adch;
        unsigned char adcl;
        unsigned char adcsra;
        unsigned char adcsrb;
        unsigned char admux;
        AvrDevice *core;
        HWAdmux *mux;
        HWARef *aref;
        HWIrqSystem *irqSystem;
        unsigned int irqVec;

        bool adchLocked;
        int adSample;
        int adMuxConfig;
        int prescaler;
        int prescalerSelect;
        int conversionState;
        bool firstConversion;
        AnalogSignalChange *notifyClient;

        enum T_State {
            IDLE,
            INIT,
            RUNNING,
        } state;

        enum {
            ADEN  = 0x80,
            BIN   = 0x80,
            ADSC  = 0x40,
            ACME  = 0x40,
            ADFR  = 0x20,
            ADATE = 0x20,
            ADLAR = 0x20,
            IPR   = 0x20,
            ADIF  = 0x10,
            ADIE  = 0x08,
            ADPS  = 0x07,
            ADTS  = 0x07
        };

        bool IsPrescalerClock(void);
        bool IsFreeRunning(void);
        virtual int GetTriggerSource(void);
        int ConversionBipolar(float value, float ref);
        int ConversionUnipolar(float value, float ref);

    public:
        enum {
            AD_4433, //!< ADC type 4433: ADC on at90s/l4433
            AD_M8,   //!< ADC type M8: ADC on atmega8
            AD_M16,  //!< ADC type M16: ADC on atmega16 and atmega32
            AD_M64,  //!< ADC type M64: ADC on atmega64
            AD_M128, //!< ADC type M16: ADC on atmega128
            AD_M48,  //!< ADC type M48: ADC on atmega48/88/168/328
            AD_M164, //!< ADC type M164: ADC on atmega164/324/644/1284 and at90can32/64/128
            AD_T25   //!< ADC type T25: ADC on attiny25/45/85
        };

        IOReg<HWAd> adch_reg,
                    adcl_reg,
                    adcsra_reg,
                    adcsrb_reg,
                    admux_reg;

        HWAd(AvrDevice *c, int _typ, HWIrqSystem *i, unsigned int iv, HWAdmux *a, HWARef *r);
        virtual ~HWAd() { mux->UnregisterNotifyClient(); }

        unsigned int CpuCycle();

        unsigned char GetAdch(void);
        unsigned char GetAdcl(void);
        unsigned char GetAdcsrA(void) { return adcsra; }
        unsigned char GetAdcsrB(void) { return adcsrb; }
        unsigned char GetAdmux(void) { return admux; }
        void SetAdcsrA(unsigned char);
        void SetAdcsrB(unsigned char);
        void SetAdmux(unsigned char val);
        void Reset(void);
        void ClearIrqFlag(unsigned int vec);

        // interface for notify signal change in multiplexer
        void NotifySignalChanged(void);

        // interface for analog comparator
        //! Check, if ADC is enabled
        bool IsADEnabled(void) { return (adcsra & ADEN) == ADEN; }
        //! Check, if ACME bit is set, return false, if not available
        bool IsSetACME(void) { return (adcsrb & ACME) == ACME; }
        //! Get analog value from ADC multiplexer
        float GetADMuxValue(float vcc) { return mux->GetValueAComp(admux, vcc); }
        //! Register analog comparator for notification of multiplexer signal change
        void RegisterNotifyClient(AnalogSignalChange *client) { notifyClient = client; }
        //! Unregister client for signal change notification
        void UnregisterNotifyClient(void) { notifyClient = 0; }
};

/** Analog-digital converter (ADC) with trigger sources in SFIOR register */
class HWAd_SFIOR: public HWAd, public IOSpecialRegClient {

    protected:
        IOSpecialReg* sfior_reg;
        int adts;

    public:
        HWAd_SFIOR(AvrDevice *c, int _typ, HWIrqSystem *i, unsigned int iv, HWAdmux *a, HWARef *r, IOSpecialReg *s);

        void Reset(void) { HWAd::Reset(); adts = 0; }

        unsigned char set_from_reg(const IOSpecialReg* reg, unsigned char nv);
        unsigned char get_from_client(const IOSpecialReg* reg, unsigned char v) { return v; }

        virtual int GetTriggerSource(void) { return adts; }
};

#endif
