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

#include <limits.h> //use MAX_INT
#include "hwad.h"

#include "irqsystem.h"
#include "avrerror.h"

#define MUX0 0x01
#define MUX1 0x02
#define MUX2 0x04
#define ADCBG 0x40 //currently not supported

using namespace std;

HWAdmux::HWAdmux(
   AvrDevice* c,
   Pin*  _ad0,
   Pin*  _ad1,
   Pin*  _ad2, 
   Pin*  _ad3,
   Pin*  _ad4,
   Pin*  _ad5,
   Pin*  _ad6,
   Pin*  _ad7
) : 
    Hardware(c),
    TraceValueRegister(c, "ADMUX"),
    core(c),
    admux_reg(this, "ADMUX", this, &HWAdmux::GetAdmux, &HWAdmux::SetAdmux)
{
    ad[0]=_ad0;
    ad[1]=_ad1;
    ad[2]=_ad2;
    ad[3]=_ad3;
    ad[4]=_ad4;
    ad[5]=_ad5;
    ad[6]=_ad6;
    ad[7]=_ad7;

    Reset();
}

void HWAdmux::Reset() {
    admux=0;
}

void HWAdmux::SetAdmux(unsigned char val) {
    admux=val;
}

unsigned char HWAdmux::GetAdmux() {
    return admux;
}

int HWAdmux::GetMuxOutput() {

    int   pin = admux&(MUX2|MUX1|MUX0);
    Pin*  p = ad[pin];
    if(!p){
	cerr << "HWAdmux::GetMuxOutput null pin on " << pin << endl;
        return 0;
    }
    return p->GetAnalog();
}

//---------------------------------------------------------------------------------
#define ADEN 0x80
#define ADSC 0x40
#define ADFR 0x20
#define ADIF 0x10
#define ADIE 0x08
#define ADPS2 0x04
#define ADPS1 0x02
#define ADPS0 0x01
#define ADLAR (1<<5)
 
HWAd::HWAd( AvrDevice *c, HWAdmux *a, HWIrqSystem *i, Pin& _aref, unsigned int iv) :
    Hardware(c),
    TraceValueRegister(c, "AD"),
    core(c),
    admux(a),
    irqSystem(i),
    aref(_aref),
    irqVec(iv),
    adch_reg(this, "ADCH",  this, &HWAd::GetAdch, 0),
    adcl_reg(this, "ADCL",  this, &HWAd::GetAdcl, 0),
    adcsr_reg(this, "ADCSR", this, &HWAd::GetAdcsr, &HWAd::SetAdcsr)
{
    irqSystem->DebugVerifyInterruptVector(irqVec, this);
    core->AddToCycleList(this);
//    irqSystem->RegisterIrqPartner(this, iv);

    Reset();
}

void HWAd::Reset() {
    adcsr=0;
    state=IDLE;
    prescaler=0;
    clk=0;
    usedBefore=false;
    adchLocked=false;
}

unsigned char HWAd::GetAdch() { adchLocked=false; return adch; }
unsigned char HWAd::GetAdcl() { adchLocked=true; return adcl; }
unsigned char HWAd::GetAdcsr() {
	return adcsr;
	}

void HWAd::SetAdcsr(unsigned char val) {
    unsigned char old=adcsr&(ADIF|ADSC);
    if (val & ADIF ) old&= ~ADIF; //clear IRQ Flag if set in val
    adcsr= old|(val& (~(ADIF)));

    if ((adcsr&(ADIE|ADIF))==(ADIE|ADIF)) {
        irqSystem->SetIrqFlag(this, irqVec);
    } else {
        irqSystem->ClearIrqFlag(irqVec);
    }
}


void HWAd::ClearIrqFlag(unsigned int vector){
    if (vector==irqVec) {
        adcsr&=~ADIF;
        irqSystem->ClearIrqFlag( irqVec);
    }
}


unsigned int HWAd::CpuCycle() {
    if (adcsr&ADEN) { //ad is enabled
        prescaler++;
        if (prescaler>=128) prescaler=0;

        //ATTENTION: clk runs 2 times faster then clock cycle in spec
        //we need the half clk for getting the analog values 
        //by cycle 1.5 as noted in avr spec, this means cycle 3 in this modell!

        unsigned char oldClk=clk;

        switch ( adcsr & ( ADPS2|ADPS1|ADPS0)) { //clocking with prescaler
            case 0: 
            case ADPS0:
                if ((prescaler%1)==0) clk++;
                break;

            case ADPS1:
                if ((prescaler%2)==0) clk++;
                break;

            case (ADPS1|ADPS0):
                if ((prescaler%4)==0) clk++;
                break;

            case (ADPS2):
                if ((prescaler%8)==0) clk++;
                break;

            case (ADPS2|ADPS0):
                if ((prescaler%16)==0) clk++;
                break;

            case (ADPS2|ADPS1):
                if ((prescaler%32)==0) clk++;
                break;

            case (ADPS2|ADPS1|ADPS0):
                if ((prescaler%64)==0) clk++;
                break;

        } //end of switch prescaler selection

        if (clk!=oldClk) { //the clk has changed

            switch (state) {
                case IDLE:
                    clk=0;
                    if (adcsr&ADSC) { //start a conversion
                        if (usedBefore) {
                            state=RUNNING;
                        } else {
                            state=INIT;
                        }
                    }
                    break;

                case INIT:
                    //we only wait 
                    if (clk==13*2) {
                        state = RUNNING;
                        clk=1*2; //we goes 1 clk ahead while waiting only 12 cycles in real
                        //only corrected while avr spec say : after 13 cycles... 
                        //normaly that can also be done after 12 cycles and start a
                        //14 cycle long normal run... but this is here spec conform :-)
                        usedBefore=true;
                    }
                    break;

                case RUNNING:
                    if (clk==1.5*2) { //sampling
                        adSample= admux->GetMuxOutput();
                        int adref= aref.GetAnalog();
                        if (adSample>adref) adSample=adref;
                        if (adref==0) {
                            adSample=INT_MAX;
                        } else {
                            adSample= (int)((float)adSample/(float)adref*INT_MAX);
                        }

                    } else if ( clk== 13*2) {
                        //calc sample to go to 10 bit value
			if(admux->GetAdmux() & ADLAR){
				adSample <<= (16-10); // Left-justify sample
			}

                        if (adchLocked) {
                            if (core->trace_on) {
                                traceOut<< "AD-Result lost adch is locked!" << endl;
                            } else {
                                cerr << "AD-Result lost adch is locked!" << endl;
                            }
                        } else { //adch is unlocked
                            adch=adSample>>8;
                        }
                        adcl=adSample&0xff;
                        adcsr|=ADIF;        //set irq flag (conversion complete)
                        if ((adcsr&(ADIE|ADIF))==(ADIE|ADIF)) {
                            irqSystem->SetIrqFlag(this, irqVec);
                        }

                        if (adcsr& ADFR) { //start again and state is running again
                            clk=0;
                        } else {
                            adcsr&=~ADSC;    //not free running->clear ADSC Bit
                        }
                    } else if (clk==14*2) {
                        clk=0; 
                        state = IDLE;
                    }
                    break;

            } // end of switch state




        }

    } else { //ad not enabled
        prescaler=0;
        clk=0;
    }

    return 0;
}

