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

#include "hwad.h"
#include "irqsystem.h"
#include "avrerror.h"

HWARefPin::HWARefPin(AvrDevice *_core):
    HWARef(_core),
    aref_pin() {
    core->RegisterPin("AREF", &aref_pin);
}

float HWARefPin::GetRefValue(int select, float vcc) {
    return aref_pin.GetAnalogValue(vcc);
}

HWARef4::HWARef4(AvrDevice *_core, int _type):
    HWARefPin(_core),
    refType(_type) { }

float HWARef4::GetRefValue(int select, float vcc) {
    switch((select >> 6) & 0x3) {
        case 0:
            return aref_pin.GetAnalogValue(vcc);

        case 1:
            return vcc;

        case 2:
            if(refType != REFTYPE_BG3) {
                avr_warning("ADC reference select invalid");
                return 0.0;
            }
            return core->v_bandgap.GetRawAnalog();

        case 3:
            if(refType == REFTYPE_BG4)
                return core->v_bandgap.GetRawAnalog();
            return 2.56;
    }
    // should not come here
    return 0.0;
}

float HWARef8::GetRefValue(int select, float vcc) {
    switch(((select >> 6) & 0x3) | ((select >> 2) & 0x4)) {
        case 0:
        case 4:
            return vcc;

        case 1:
        case 5:
            return aref_pin->GetAnalogValue(vcc);

        case 2:
            return core->v_bandgap.GetRawAnalog();

        case 3:
            avr_warning("ADC reference select invalid");
            return 0.0;

        case 6:
        case 7:
            return 2.56;
    }
    // should not come here
    return 0.0;
}

void HWAdmux::SetMuxSelect(int select) {
    int oldSelect = muxSelect;
    muxSelect = select & 0x7; // only values 0..7 allowed!
    if(numPins < 6)
        muxSelect &= 0x3; // only values 0..3 allowed!
    if((notifyClient != NULL) && (oldSelect != muxSelect))
        notifyClient->NotifySignalChanged();
}

void HWAdmux::PinStateHasChanged(Pin* p) {
    Pin *selected = ad[muxSelect];
    if((notifyClient != NULL) && (selected == p))
        notifyClient->NotifySignalChanged();
}

HWAdmux6::HWAdmux6(AvrDevice* c, Pin*  _ad0,
                                 Pin*  _ad1,
                                 Pin*  _ad2,
                                 Pin*  _ad3,
                                 Pin*  _ad4,
                                 Pin*  _ad5):
    HWAdmux(c, 6) {
    ad[0] = _ad0;
    _ad0->RegisterCallback(this);
    ad[1] = _ad1;
    _ad1->RegisterCallback(this);
    ad[2] = _ad2;
    _ad2->RegisterCallback(this);
    ad[3] = _ad3;
    _ad3->RegisterCallback(this);
    ad[4] = _ad4;
    _ad4->RegisterCallback(this);
    ad[5] = _ad5;
    _ad5->RegisterCallback(this);
    ad[6] = NULL;
    ad[7] = NULL;
}

float HWAdmux6::GetValue(int select, float vcc) {
    if(core->fuses->GetFuseBit(3) && ((select & 0x40) == 0x40))
        return 1.22; // ADCBG is set and BODEN-fuse is programmed (at90x4433 only)
    int adChannel = select & 0x07; // bit 2:0 is multiplexer selector
    if(adChannel >= numPins) {
        avr_warning("adc multiplexer has selected non existent channel %d", adChannel);
        return 0.0;
    }
    return ad[adChannel]->GetAnalogValue(vcc);
}

HWAdmuxM8::HWAdmuxM8(AvrDevice* c, Pin*  _ad0,
                                   Pin*  _ad1,
                                   Pin*  _ad2,
                                   Pin*  _ad3,
                                   Pin*  _ad4,
                                   Pin*  _ad5,
                                   Pin*  _ad6,
                                   Pin*  _ad7):
    HWAdmux(c, 8) {
    ad[0] = _ad0;
    _ad0->RegisterCallback(this);
    ad[1] = _ad1;
    _ad1->RegisterCallback(this);
    ad[2] = _ad2;
    _ad2->RegisterCallback(this);
    ad[3] = _ad3;
    _ad3->RegisterCallback(this);
    ad[4] = _ad4;
    _ad4->RegisterCallback(this);
    ad[5] = _ad5;
    _ad5->RegisterCallback(this);
    ad[6] = _ad6;
    _ad6->RegisterCallback(this);
    ad[7] = _ad7;
    _ad7->RegisterCallback(this);
}

HWAdmuxM8::HWAdmuxM8(AvrDevice* c, Pin*  _ad0,
                                   Pin*  _ad1,
                                   Pin*  _ad2,
                                   Pin*  _ad3):
    HWAdmux(c, 4) {
    ad[0] = _ad0;
    _ad0->RegisterCallback(this);
    ad[1] = _ad1;
    _ad1->RegisterCallback(this);
    ad[2] = _ad2;
    _ad2->RegisterCallback(this);
    ad[3] = _ad3;
    _ad3->RegisterCallback(this);
    ad[4] = NULL;
    ad[5] = NULL;
    ad[6] = NULL;
    ad[7] = NULL;
}

float HWAdmuxM8::GetValue(int select, float vcc) {
    int adChannel = select & 0x0f; // bit 3:0 is multiplexer selector
    if(adChannel == 15)
        return 0.0;                            // GND channel
    if(adChannel == 14)
        return core->v_bandgap.GetRawAnalog(); // BG channel
    if(adChannel >= numPins) {
        avr_warning("adc multiplexer has selected non existent channel %d", adChannel);
        return 0.0;
    }
    return ad[adChannel]->GetAnalogValue(vcc);
}

float HWAdmuxM8::GetValueAComp(int select, float vcc) {
    int adChannel = select & 0x07; // bit 2:0 is multiplexer selector for analog comparator
    return ad[adChannel]->GetAnalogValue(vcc);
}

HWAdmuxM16::HWAdmuxM16(AvrDevice* c, Pin*  _ad0,
                                     Pin*  _ad1,
                                     Pin*  _ad2,
                                     Pin*  _ad3,
                                     Pin*  _ad4,
                                     Pin*  _ad5,
                                     Pin*  _ad6,
                                     Pin*  _ad7):
    HWAdmuxM8(c, _ad0, _ad1, _ad2, _ad3, _ad4, _ad5, _ad6, _ad7) { }

float HWAdmuxM16::GetValue(int select, float vcc) {
    int adChannel = select & 0x1f; // bit 4:0 is multiplexer selector
    if(adChannel == 31)
        return 0.0;                            // GND channel
    if(adChannel == 30)
        return core->v_bandgap.GetRawAnalog(); // BG channel
    if(adChannel < 8)
        return ad[adChannel]->GetAnalogValue(vcc); // single channel
    if(adChannel >= 24) {
        float neg = ad[2]->GetAnalogValue(vcc); // channel 2 is negative difference channel, gain is 1
        return ad[adChannel - 24]->GetAnalogValue(vcc) - neg;
    }
    if(adChannel >= 16) {
        float neg = ad[1]->GetAnalogValue(vcc); // channel 1 is negative difference channel, gain is 1
        return ad[adChannel - 16]->GetAnalogValue(vcc) - neg;
    }
    if((adChannel == 8) || (adChannel == 9) || (adChannel == 12) || (adChannel == 13)) {
        float neg = ad[(adChannel > 9) ? 2 : 0]->GetAnalogValue(vcc); // channel 0/2 is negative difference channel, gain is 10
        if(adChannel == 8)
            return (ad[0]->GetAnalogValue(vcc) - neg) * 10.0;
        if(adChannel == 9)
            return (ad[1]->GetAnalogValue(vcc) - neg) * 10.0;
        if(adChannel == 12)
            return (ad[2]->GetAnalogValue(vcc) - neg) * 10.0;
        return (ad[3]->GetAnalogValue(vcc) - neg) * 10.0;
    }
    float neg = ad[(adChannel > 11) ? 2 : 0]->GetAnalogValue(vcc); // channel 0/2 is negative difference channel, gain is 200
    if(adChannel == 10)
        return (ad[0]->GetAnalogValue(vcc) - neg) * 200.0;
    if(adChannel == 11)
        return (ad[1]->GetAnalogValue(vcc) - neg) * 200.0;
    if(adChannel == 14)
        return (ad[2]->GetAnalogValue(vcc) - neg) * 200.0;
    return (ad[3]->GetAnalogValue(vcc) - neg) * 200.0;
}

bool HWAdmuxM16::IsDifferenceChannel(int select) {
    int adChannel = select & 0x1f; // bit 4:0 is multiplexer selector
    return (adChannel >= 8) && (adChannel < 30);
}

HWAdmuxT25::HWAdmuxT25(AvrDevice* c, Pin*  _ad0,
                                     Pin*  _ad1,
                                     Pin*  _ad2,
                                     Pin*  _ad3):
    HWAdmuxM8(c, _ad0, _ad1, _ad2, _ad3) { }

float HWAdmuxT25::GetValue(int select, float vcc) {
    int adChannel = select & 0xf; // bit 3:0 is multiplexer selector
    if(adChannel == 15)
        return 0.322;                          // Temp. sensor, temperature about 25°C
    if(adChannel == 14) {
        avr_warning("adc multiplexer has selected non existent channel %d", adChannel);
        return 0.0;                            // not available
    }
    if(adChannel == 13)
        return 0.0;                            // GND channel
    if(adChannel == 12)
        return core->v_bandgap.GetRawAnalog(); // BG channel
    if(adChannel < 4)
        return ad[adChannel]->GetAnalogValue(vcc); // single channel
    // all other are difference channel
    if(adChannel == 4)
        return (ad[2]->GetAnalogValue(vcc) - ad[2]->GetAnalogValue(vcc)) * 1.0;
    if(adChannel == 5)
        return (ad[2]->GetAnalogValue(vcc) - ad[2]->GetAnalogValue(vcc)) * 20.0;
    if(adChannel == 6)
        return (ad[2]->GetAnalogValue(vcc) - ad[3]->GetAnalogValue(vcc)) * 1.0;
    if(adChannel == 7)
        return (ad[2]->GetAnalogValue(vcc) - ad[3]->GetAnalogValue(vcc)) * 20.0;
    if(adChannel == 8)
        return (ad[0]->GetAnalogValue(vcc) - ad[0]->GetAnalogValue(vcc)) * 1.0;
    if(adChannel == 9)
        return (ad[0]->GetAnalogValue(vcc) - ad[0]->GetAnalogValue(vcc)) * 20.0;
    if(adChannel == 10)
        return (ad[0]->GetAnalogValue(vcc) - ad[1]->GetAnalogValue(vcc)) * 1.0;
    // adChannel == 11
    return (ad[0]->GetAnalogValue(vcc) - ad[1]->GetAnalogValue(vcc)) * 20.0;
}

bool HWAdmuxT25::IsDifferenceChannel(int select) {
    int adChannel = select & 0xf; // bit 3:0 is multiplexer selector
    return (adChannel >= 4) && (adChannel < 12);
}

HWAd::HWAd(AvrDevice *c, int _typ, HWIrqSystem *i, unsigned int iv, HWAdmux *a, HWARef *r):
    Hardware(c),
    TraceValueRegister(c, "AD"),
    adType(_typ),
    core(c),
    mux(a),
    aref(r),
    irqSystem(i),
    irqVec(iv),
    notifyClient(NULL),
    adch_reg(this, "ADCH",  this, &HWAd::GetAdch, 0),
    adcl_reg(this, "ADCL",  this, &HWAd::GetAdcl, 0),
    adcsra_reg(this, "ADCSRA", this, &HWAd::GetAdcsrA, &HWAd::SetAdcsrA),
    adcsrb_reg(this, "ADCSRB", this, &HWAd::GetAdcsrB, &HWAd::SetAdcsrB),
    admux_reg(this, "ADMUX", this, &HWAd::GetAdmux, &HWAd::SetAdmux) {
    mux->RegisterNotifyClient(this);
    irqSystem->DebugVerifyInterruptVector(irqVec, this);
    core->AddToCycleList(this);

    Reset();
}

void HWAd::Reset(void) {
    adcsra = adcsrb = 0;
    adch = 0;
    adcl = 0;
    admux = adMuxConfig = 0;
    state = IDLE;
    prescaler = 0;
    prescalerSelect = 0;
    conversionState = 0;
    firstConversion = true;
    adchLocked = false;
}

void HWAd::NotifySignalChanged(void) {
    if((notifyClient != NULL) && !IsADEnabled())
        notifyClient->NotifySignalChanged();
}

unsigned char HWAd::GetAdch(void) {
    adchLocked = false;
    return adch;
}

unsigned char HWAd::GetAdcl(void) {
    adchLocked = true;
    return adcl;
}

void HWAd::SetAdcsrA(unsigned char val) {
    bool enabled = (adcsra & ADEN) == ADEN;
    // clear IRQ Flag if set in val, otherwise do not overwrite ADIF
    if((val & ADIF) == ADIF)
        val &= ~ADIF;
    else if((adcsra & ADIF) == ADIF)
        val |= ADIF;
    // if ADSC is set, don't clear it till end of conversion
    if((adcsra & ADSC) == ADSC)
        val |= ADSC;
    // store value
    adcsra = val;

    // set prescaler selection
    prescalerSelect = adcsra & ADPS;

    // set first conversion flag (first conversion after enable ADC)
    if(!enabled && ((adcsra & ADEN) == ADEN))
        firstConversion = true;

    // handle interrupt, if fresh enabled
    if((adcsra & (ADIE | ADIF)) == (ADIE | ADIF))
        irqSystem->SetIrqFlag(this, irqVec);
    else
        irqSystem->ClearIrqFlag(irqVec);

    // handle connection to analog comparator
    NotifySignalChanged();
}

void HWAd::SetAdcsrB(unsigned char val) {
    if(adType == AD_T25)
        val &= 0xe7; // reset bit4,3
    else if(adType == AD_M64)
        val &= 0x07; // reset bit7,6,5,4,3
    else
        val &= 0x47; // reset bit7,5,4,3
    adcsrb = val;

    // handle connection to analog comparator
    NotifySignalChanged();
}

void HWAd::SetAdmux(unsigned char val) {
    if(adType == AD_4433)
        val &= 0x47; // reset bit7,5,4,3
    else if((adType == AD_M8) || (adType == AD_M48))
        val &= 0xef; // reset bit4
    admux = val;
    mux->SetMuxSelect((int)admux);
}

void HWAd::ClearIrqFlag(unsigned int vector){
    if(vector == irqVec) {
        adcsra &= ~ADIF;
        irqSystem->ClearIrqFlag(irqVec);
    }
}

/* ATTENTION: prescaler clock here runs 2 times faster then clock cycle in spec.
   we need the half clock for getting the analog values
   by cycle 1.5 as noted in AVR spec., this means cycle 3 in this model! */
bool HWAd::IsPrescalerClock(void) {
    if((adcsra & ADEN) == 0) {
        // ADC disabled, prescaler remains in reset
        prescaler = 0;
        return false;
    }

    // ADC enabled, prescaler counts continuously
    prescaler++;
    if(prescaler >= 64) // counter size = 7bit, but we count with double speed
        prescaler = 0;

    switch(prescalerSelect) {
        case 0:
        case 1: // CLKx2
            return true;

        case 2: // CLKx4
            return (prescaler % 2) == 0;

        case 3: // CLKx8
            return (prescaler % 4) == 0;

        case 4: // CLKx16
            return (prescaler % 8) == 0;

        case 5: // CLKx32
            return (prescaler % 16) == 0;

        case 6: // CLKx64
            return (prescaler % 32) == 0;

        case 7: // CLKx128
            return (prescaler % 64) == 0;

    }
    // should not happen
    return false;
}

int HWAd::GetTriggerSource(void) {
    return adcsrb & ADTS;
}

bool HWAd::IsFreeRunning(void) {
    if((adType == AD_4433) || (adType == AD_M8) || (adType == AD_M128))
        // only freerunning mode, no autotrigger modes
        return (adcsra & ADFR) == ADFR;
    // all other have ADATE flag and autotrigger modes
    if((adcsra & ADATE) == ADATE) {
        // does have ADTS[2:0] in SFIOR register or in ADCSRB register
        return GetTriggerSource() == 0; // mode 0 is freerunning mode
    }
    // not freerunning
    return false;
}

int HWAd::ConversionBipolar(float value, float ref) {
    // max value is 511 (positive), min value is -512 (negative)
    int adcmax = (1 << 9) - 1;
    int adcmin = -(1 << 9);
    // delimiting range -Vref to Vref
    if(value > ref)
        value = ref;
    else if(value < -ref)
        value = -ref;
    // convert to int
    if(ref == 0.0) {
        if(value < 0.0)
            return adcmin;
        else
            return adcmax;
    } else
        return ((int)((value * (adcmax + 1)) / ref)) & 0x3ff;
}

int HWAd::ConversionUnipolar(float value, float ref) {
    // max value is 1023, min value is 0
    int adcmax = (1 << 10) - 1;
    // delimiting range GND to Vref
    if(value > ref)
        value = ref;
    else if(value < 0.0)
        value = 0.0;
    // convert to int
    if(ref == 0.0)
        return adcmax;
    else
        return (int)((value * (adcmax + 1)) / ref);

}

unsigned int HWAd::CpuCycle() {

    if(IsPrescalerClock()) { // prescaler clock event

        conversionState++;

        switch (state) {

            case IDLE:
                conversionState = 0;
                if((adcsra & ADSC) == ADSC) { //start a conversion
                    adMuxConfig = admux; // buffer ADMUX state
                    if(firstConversion) {
                        state = INIT;
                        firstConversion = false;
                    } else
                        state = RUNNING;
                }
                break;

            case INIT:
                // we have to wait 13 clocks extra
                if (conversionState == (13 * 2)) {
                    state = RUNNING;
                    conversionState = (1 * 2);
                    /* we goes 1 clock ahead while waiting only 12 cycles in real
                       only corrected while avr spec say : after 13 cycles...
                       Normally that can also be done after 12 cycles and start a
                       14 cycle long normal run... but this is here spec conform :-) */
                }
                break;

            case RUNNING:
                if(conversionState == ((1 * 2) + 1)) { // sample time
                    float vcc = core->v_supply.GetRawAnalog();
                    float adref = aref->GetRefValue(adMuxConfig, vcc);
                    float muxval = mux->GetValue(adMuxConfig, vcc);
                    if(mux->IsDifferenceChannel(adMuxConfig)) {
                        if(adType == AD_T25) {
                            if((adcsrb & BIN) == BIN)
                                adSample = ConversionBipolar(muxval, adref);
                            else {
                                if((adcsrb & IPR) == IPR)
                                    adSample = ConversionUnipolar(-muxval, adref);
                                else
                                    adSample = ConversionUnipolar(muxval, adref);
                            }
                        } else
                            adSample = ConversionBipolar(muxval, adref);
                    } else
                        adSample = ConversionUnipolar(muxval, adref);
                } else if(conversionState == (13 * 2)) {
                    // calculate sample to go to 10 bit value
                    if((admux & ADLAR) == ADLAR)
                        adSample <<= (16 - 10); // left-justify sample
                    // set ADCL and ADCH
                    if(adchLocked) {
                        // TODO: how to rewrite this, is traceOut right? Replace output to stderr.
                        if(core->trace_on)
                            traceOut << "ADC result lost, adch is locked!" << std::endl;
                        else
                            std::cerr << "AD-Result lost adch is locked!" << std::endl;
                    } else // adch is unlocked
                        adch = adSample >> 8;
                    adcl = adSample & 0xff;

                    // set irq flag (conversion complete) and trigger interrupt, if enabled
                    adcsra |= ADIF;
                    if((adcsra & (ADIE | ADIF)) == (ADIE | ADIF))
                        irqSystem->SetIrqFlag(this, irqVec);

                    if(IsFreeRunning()) { // free running mode: start again and state is running again
                        conversionState = 0;
                        adMuxConfig = admux; // buffer ADMUX state
                    } else
                        adcsra &= ~ADSC;    // not free running -> clear ADSC Bit
                } else if(conversionState == (14 * 2)) {
                    conversionState = 0;
                    state = IDLE;
                }
                break;

        } // end of switch state
    }

    return 0;
}

HWAd_SFIOR::HWAd_SFIOR(AvrDevice *c, int _typ, HWIrqSystem *i, unsigned int iv, HWAdmux *a, HWARef *r, IOSpecialReg *s):
    HWAd(c, _typ, i, iv, a, r),
    sfior_reg(s) {
    adts = 0;
    sfior_reg->connectSRegClient(this);
}

unsigned char HWAd_SFIOR::set_from_reg(const IOSpecialReg* reg, unsigned char nv) {
    adts = (nv >> 5) & 0x7;
    return nv;
}

// EOF
