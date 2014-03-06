/*
 ****************************************************************************
 *
 * simulavr - A simulator for the Atmel AVR family of microcontrollers.
 * Copyright (C) 2001, 2002, 2003   Klaus Rudolph
 * Copyright (C) 2009 Onno Kortmann
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

#include <assert.h>
#include <stdio.h>
#include "hwspi.h"
#include "flash.h"
#include "avrdevice.h"
#include "traceval.h"
#include "irqsystem.h"
#include "avrerror.h"

//configuration
#define SPIE 0x80
#define SPE  0x40
#define DORD 0x20  ///< "When the DORD bit is written to one, the LSB of the data word is transmitted first."
#define MSTR 0x10
#define CPOL 0x08  ///< "When this bit is written to one, SCK is high when idle."
#define CPHA 0x04  ///< When this bit is written to one, output is setup at leading edge and input is sampled trailing edge.
#define SPR1 0x02
#define SPR0 0x01

//status
#define SPIF 0x80
#define WCOL 0x40
#define SPI2X 0x01  //only on mega devices speed x 2


/* SPI verbosity level
   FIXME: Make this configurable through the command line interface. */
#define SPI_VERBOSE 0

using namespace std;
void HWSpi::spdr_access() {
    if (spsr_read) {
    // if status is read with SPIF == 1
    //we can remove the SPIF and WCOL flag after reading
    //data register
    spsr&=~(SPIF|WCOL);
    spsr_read=false;
    }
}

unsigned char HWSpi::GetSPDR() {
    spdr_access();
    return data_read;
}

unsigned char HWSpi::GetSPSR() { 
    spsr_read=true;
    return spsr;
}

unsigned char HWSpi::GetSPCR() {
    return spcr;
}

void HWSpi::SetSPDR(unsigned char val) {
    spdr_access();
    data_write=val;
    if (spcr & MSTR) { // mster mode?
        if (bitcnt<8) {
            spsr|=WCOL; // not yet ready -> Write Collision
        } else {
            bitcnt=0;
            finished=false;
            clkcnt=0;
        }
    }
}

void HWSpi::updatePrescaler() {
    int fac2x=(spsr&SPI2X) ? 1 : 2;
    switch (spcr & (SPR1|SPR0)) {
    case 0: clkdiv=1; break;
    case SPR0: clkdiv=4; break;
    case SPR1: clkdiv=16; break;
    case SPR1|SPR0: clkdiv=32; break;
    }
    clkdiv*=fac2x;
}

void HWSpi::SetSPSR(unsigned char val) {
    if (mega_mode) {
        spsr&=~SPI2X;
        spsr|=val&SPI2X;
        updatePrescaler();
    } else {
        ((core->trace_on) ?
            (traceOut) : (cerr))
            << "spsr is read only! (0x" << hex << core->PC << " =  " <<
            core->Flash->GetSymbolAtAddress(core->PC) << ")" << endl;
    }
}


void HWSpi::SetSPCR(unsigned char val) { 
    spcr=val;
    if ( spcr & SPE) { //SPI is enabled
        core->AddToCycleList(this);
        if (spcr & MSTR) { //master
            MISO.SetUseAlternateDdr(1);
            MISO.SetAlternateDdr(0); //always input
            MOSI.SetUseAlternatePortIfDdrSet(1);

            /* according to the graphics in the atmega8 datasheet, p.132
           (10/06), MOSI is high when idle. FIXME: check whether
           this applies to real hardware. */
            MOSI.SetAlternatePort(1);
            SCK.SetAlternatePort(spcr & CPOL);
            SCK.SetUseAlternatePortIfDdrSet(1);
            assert(SCK.GetPin().outState == ((spcr & CPOL) ? Pin::HIGH : Pin::LOW));
            assert(SCK.GetPin().outState == ((spcr & CPOL) ? Pin::HIGH : Pin::LOW));
        } else { //slave
            MISO.SetUseAlternatePortIfDdrSet(1);
            MOSI.SetUseAlternateDdr(1);
            MOSI.SetAlternateDdr(0);
            SCK.SetUseAlternateDdr(1);
            SCK.SetAlternateDdr(0);
            SS.SetUseAlternateDdr(1);
            SS.SetAlternateDdr(0);
        } 
    } else { //Spi is off so unset alternate pin functions

        /* FIXME: Check whether these will be really tied
       to reset state as long as the SPI is off. Check
       the switch on/off behaviour of the SPI interface! */
        bitcnt=8;
        finished=false;
        core->RemoveFromCycleList(this);
        MOSI.SetUseAlternatePortIfDdrSet(0);
        MISO.SetUseAlternatePortIfDdrSet(0);
        SCK.SetUseAlternatePortIfDdrSet(0);
        MOSI.SetUseAlternateDdr(0);
        MISO.SetUseAlternateDdr(0);
        SCK.SetUseAlternateDdr(0);
        SS.SetUseAlternateDdr(0);
    }
    updatePrescaler();
}


HWSpi::HWSpi(AvrDevice *_c,
         HWIrqSystem *_irq,
         PinAtPort mosi,
         PinAtPort miso,
         PinAtPort sck,
         PinAtPort ss,
         unsigned int ivec,
         bool mm) : 
    Hardware(_c), TraceValueRegister(_c, "SPI"),
    core(_c), irq(_irq),
    MOSI(mosi), MISO(miso), SCK(sck), SS(ss),
    irq_vector(ivec), mega_mode(mm),
    spdr_reg(this, "SPDR", this, &HWSpi::GetSPDR, &HWSpi::SetSPDR),
    spsr_reg(this, "SPSR", this, &HWSpi::GetSPSR, &HWSpi::SetSPSR),
    spcr_reg(this, "SPCR", this, &HWSpi::GetSPCR, &HWSpi::SetSPCR)
{
    irq->DebugVerifyInterruptVector(ivec, this);
    bitcnt=8;
    finished=false;

    trace_direct(this, "shift_in", &shift_in);
    trace_direct(this, "data_read", &data_read);
    trace_direct(this, "data_write", &data_write);
    trace_direct(this, "sSPSR", &spsr);
    trace_direct(this, "sSPCR", &spcr);
    Reset();
}

void HWSpi::Reset() {
    SetSPCR(0);
    spsr=0;
    data_write=data_read=shift_in=0;
}

void HWSpi::ClearIrqFlag(unsigned int vector) {
    if (vector==irq_vector) {
        spsr&=~SPIF;
        irq->ClearIrqFlag(irq_vector);
    } else {
        cerr << "WARNING: There is HWSPI called to get a irq vector which is not assigned for!?!?!?!?";
    }
}

void HWSpi::txbit(const int bitpos) {
    //  set next output bit
    PinAtPort *out=(spcr & MSTR) ? &MOSI : &MISO;
    out->SetAlternatePort(data_write&(1<<bitpos));
}

void HWSpi::rxbit(const int bitpos) {
    // sample input
    bool bit=(spcr & MSTR) ? MISO : MOSI;
    if (bit)
    shift_in|=(1<<bitpos);
}

void HWSpi::trxend() {
    if (finished) {
    finished=false;
    if (core->trace_on && SPI_VERBOSE)
        traceOut << "SPI: READ " << int(shift_in) << endl;
    /* set also data_write to allow continuous shifting
       when slave. */
    data_write=data_read=shift_in; 
                       
    spsr|=SPIF;
    if (spcr&SPIE) {
        irq->SetIrqFlag(this, irq_vector);
    }   
    spsr_read=false;
    }
}

unsigned int HWSpi::CpuCycle() {
    if ((spcr & SPE) == 0)  // active at all?
        return 0;
    int bitpos=(spcr&DORD) ? bitcnt : 7-bitcnt;
    int bitpos_prec=(spcr&DORD) ? bitcnt-1 : 8-bitcnt;
    
    if (core->trace_on && SPI_VERBOSE) {
        traceOut << "SPI: " << bitcnt << ", " << bitpos << ", " << clkcnt << endl;
    }
    
    if (spcr & MSTR) {
        /* Check whether we're externally driven into slave mode.
           FIXME: It is unclear at least from mega8 docs if this behaviour is
           also right when the SPI is inactive!*/
        if (! SS.GetDdr() && ! SS) {
            SetSPCR(spcr & ~MSTR);
            // request interrupt
            spsr |= SPIF;
            if (spcr&SPIE) {
                irq->SetIrqFlag(this, irq_vector);
            }
            bitcnt = 8; // slave and idle
            finished=false;
                clkcnt=0;
        }
        if ((clkcnt%clkdiv) == 0){ // TRX bits
            if (bitcnt < 8) {
                if (bitcnt == 0)
                    shift_in = 0;
                switch ((clkcnt/clkdiv)&1) {
                case 0:
                    // set idle clock
                    SCK.SetAlternatePort(spcr&CPOL);
                    // late phase (for last bit)?
                    if (spcr&CPHA) {
                        if (bitcnt) {
                            rxbit(bitpos_prec);
                        }
                    } else {
                        txbit(bitpos);
                    }
                    break;
                case 1:
                    // set valid clock
                    SCK.SetAlternatePort(!(spcr&CPOL));
                    if (spcr&CPHA) {
                        txbit(bitpos);
                    } else {
                        rxbit(bitpos);
                    }
                    bitcnt++;
                    break;
                }
                finished = (bitcnt==8);
            } else if (finished) {
                if (spcr&CPHA) {
                    rxbit(bitpos_prec);
                }
                trxend();
                // set idle clock
                SCK.SetAlternatePort(spcr&CPOL);
                // set idle MOSI (high if CPHA==0)
                if (!(spcr&CPHA))
                    MOSI.SetAlternatePort(1);
            }
        }
    } else {
        // possible slave mode
        if (SS) {
            // slave selected lifted-> force end of transmission
            bitcnt=8;
        } else {
            // Slave mode
            if (bitcnt == 8) {
                bitcnt = 0;
                finished = false;
                shift_in = 0;
                oldsck = SCK;
            } else {
                /* Set initial bit for CPHA==0 */
                if (!(spcr&CPHA)) {
                    txbit(bitpos);
                }
            }
            if (SCK != oldsck) { // edge detection
                bool leading = false; // leading edge clock?
                if (spcr&CPOL) {
                    // leading edge is falling edge
                    leading = ! SCK;
                } else
                    leading = SCK;

                // determine whether we should sample or setup
                bool sample = leading ^ ((spcr&CPHA)!=0);

                if (sample)
                    rxbit(bitpos);
                else
                    txbit(bitpos);

                if (!leading) {
                    bitcnt++;
                    finished = (bitcnt==8);
                }
            }
            trxend();
            oldsck = SCK;
        }
    }
    clkcnt++;
    return 0;
}

