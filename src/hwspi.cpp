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


/*TODO if reading first the spsr and after that read spdr the SPIF Flag must be cleared! */

#include "hwspi.h"
#include "avrdevice.h"
#include "trace.h"
#include "irqsystem.h"
//configuration
#define SPIE 0x80
#define SPE 0x40
#define DORD 0x20
#define MSTR 0x10
#define CPOL 0x08
#define CPHA 0x04
#define SPR1 0x02
#define SPR0 0x01
//status
#define SPIF 0x80
#define WCOL 0x40

#define SPI2X 0x01  //only on mega devices speed x 2

void HWSpi::ror(unsigned char *val) {
    unsigned int x;
    bool bit=*val&0x01;
    x=*val>>1;
    if (bit) {
        x|=0x80;
    }
    *val=x;
}

void HWSpi::rol(unsigned char *val) {
    unsigned int x;
    bool bit=*val&0x80;
    x=*val<<1;
    if (bit) {
        x|=0x01;
    }
    *val=x;
}



void HWSpi::SetSpdr(unsigned char val) { 
    spdrWrite=val;
    if (spcr& SPE) { //spi is enabled
        if (spcr&MSTR) { //we are master
            if (state!=READY) { 
                spsr|=WCOL; //Write Collision
            } else {
                state=START_AS_MASTER;
            }
        }
    }
} 

void HWSpi::SetSpsr(unsigned char val) { 
    if (trace_on) {
        traceOut << "spsr is read onyl! ";
    } else {
        cerr << "spsr is read only! ";
    }
}


void HWSpi::SetSpcr(unsigned char val) { 
    unsigned char spcrold=spcr;
    spcr=val;

    if ( ( spcr & SPE) != (spcrold & SPE) ) 
    {
        if (spcr & SPE) 
        {
            core->AddToCycleList(this);
        } else {
            core->RemoveFromCycleList(this);
        }
    }
        

    

    if ( spcr & SPE) { //SPI is enabled
        if (spcr & MSTR) { //master
            pinMiso.SetUseAlternateDdr(1);
            pinMiso.SetAlternateDdr(0); //ever input
            pinMosi.SetUseAlternatePortIfDdrSet(1);
            pinSck.SetUseAlternatePortIfDdrSet(1);
            if (spcr & CPOL) { // high idle
                pinSck.SetAlternatePort(1);
            } else {
                pinSck.SetAlternatePort(0);
            }

        } else { //slave
            pinMiso.SetUseAlternatePortIfDdrSet(1);
            pinMosi.SetUseAlternateDdr(1);
            pinMosi.SetAlternateDdr(0);
            pinSck.SetUseAlternateDdr(1);
            pinSck.SetAlternateDdr(0);
            pinSs.SetUseAlternateDdr(1);
            pinSs.SetAlternateDdr(0);
        } 
        //selecting the clockDivider

        switch (spcr & (SPR1|SPR0) ) {
            case 0: clkDiv=4; break;
            case SPR0: clkDiv=16; break;
            case SPR1: clkDiv=64; break;
            case SPR1|SPR0: clkDiv=128; break;
        } // end of switch


    } else { //Spi is off so unset alternate pin functions
        pinMosi.SetUseAlternatePortIfDdrSet(0);
        pinMiso.SetUseAlternatePortIfDdrSet(0);
        pinSck.SetUseAlternatePortIfDdrSet(0);
        pinMosi.SetUseAlternateDdr(0);
        pinMiso.SetUseAlternateDdr(0);
        pinSck.SetUseAlternateDdr(0);
        pinSs.SetUseAlternateDdr(0);
    }



}

unsigned int HWSpi::CpuCycle() {
    //check for external SS activation
    if (spcr & SPE ) { //spi is enabled
        if ( spcr & MSTR) { //we are currently master
            if ( pinSs.GetDdr()==0) { // the pin is input, so that is SS!
                if (pinSs==0) { // the /SS is set to 0, so we become a slave
                    SetSpcr(spcr&(0xff-MSTR)); //clear the Master Flag, Set all Port directions 
                    spsr|=SPIF;
                    if (spsr&SPIE) { irqSystem->SetIrqFlag(this, vectorForSpif); }
                    state=START_AS_SLAVE;
                } // end of ss is low
            } // end of pin ss is input 
        }// end of we are master

        clkCnt++;

        if ( state == READY ) {
            if (( spcr & MSTR ) ==0) { //we are slave
                if (pinSs==0) { //activate SPI for reading
                    state=START_AS_SLAVE;
                    bitCnt=0;
                }
            }
        }

        if (state==START_AS_SLAVE) state=BIT_SLAVE; //remove that state later if all is fine here

        if (state==BIT_SLAVE) {
            if (pinSs==1) { 	//break the receiving while Ss comes High
                state=READY;
            } else {

                bool sampling; //0->setup, 1->sampling
                if ( oldSck != pinSck) { //something has changed for sck
                    //bool sckVal= pinSck;
                    if ( (( spsr & CPOL ) !=0 ) ^ ( ( spsr & CPHA) !=0 ) ^ ((bool)pinSck) ) {
                        sampling=1;
                    } else {
                        sampling=0;
                    }

                    if (sampling) {
                        if (spcr & DORD) { // LSB first
                            ror(&spdrRead);
                            spdrRead&=0x7f;
                            if (pinMiso) { //pin == 1
                                spdrRead|=0x80;
                            }
                        } else {
                            rol(&spdrRead);
                            spdrRead&=0xfe;
                            if (pinMiso) { //pin == 1
                                spdrRead|=0x01;
                            }
                        }
                        bitCnt++;
                        if (bitCnt==7) { //ready
                            state= READY;
                            spsr|=SPIF;
                            if (spsr&SPIE) { irqSystem->SetIrqFlag(this, vectorForSpif); }
                            spifWeak=0; 	//after accessing the spsr the spif is weak not yet
                        } // end of all bits sampled

                    } else { //setup
                        int bitPos;
                        if (spcr & DORD) { //LSB first
                            bitPos=bitCnt;
                        } else { 			// MSB fisrt
                            bitPos=7-bitCnt;
                        }
                        bool outBit=(spdrWrite&(1<<bitPos))>>bitPos;
                        pinMiso.SetAlternatePort(outBit);
                    } //end of ?sampling
                } //end of sck changed
            } // end of else Ss==1
        } // end of BIT_SLAVE

        if (( clkCnt%(clkDiv/4))==0) {
            switch( state) {
                case READY:
                    break;

                case START_AS_MASTER:
                    {
                        clkCnt=3; //we synchronise to 0 here for following procedure
                        state=BIT_MASTER;
                        bitCnt=0;
                        int bitPos;
                        if (spcr & DORD) { //LSB first
                            bitPos=bitCnt;
                        } else { 			// MSB fisrt
                            bitPos=7-bitCnt;
                        }
                        bool outBit=(spdrWrite&(1<<bitPos))>>bitPos;
                        pinMosi.SetAlternatePort( outBit);
                    }

                    break;

                case BIT_MASTER:
                    {
                        int bitPos;
                        if (spcr & DORD) { //LSB first
                            bitPos=bitCnt;
                        } else { 			// MSB fisrt
                            bitPos=7-bitCnt;
                        }

                        bool valClkIdle= (spsr  & CPOL); 
                        bool valClkValid= !(spsr & CPOL);
                        if ( spcr & CPHA) { //==1
                            switch (clkCnt%(clkDiv/4)) {

                                case 0:	
                                    pinSck.SetAlternatePort(valClkIdle);
                                    break;

                                case 1:	
                                    break;

                                case 2: 
                                    if (spcr & DORD) { // LSB first
                                        ror(&spdrRead);
                                        spdrRead&=0x7f;
                                        if (pinMiso) { //pin == 1
                                            spdrRead|=0x80;
                                        }
                                    } else {
                                        rol(&spdrRead);
                                        spdrRead&=0xfe;
                                        if (pinMiso) { //pin == 1
                                            spdrRead|=0x01;
                                        }
                                    }
                                    pinSck.SetAlternatePort(valClkValid);


                                    break;

                                case 3:
                                    {
                                        bitCnt++;
                                        if (bitCnt==7) {
                                            state=READY;
                                            spsr|=SPIF;
                                            if (spsr&SPIE) { irqSystem->SetIrqFlag(this, vectorForSpif); }
                                            spifWeak=0;
                                            pinMosi.SetAlternatePort( 1) ; //defaults to high??
                                        } else {
                                            bool outBit=(spdrWrite&(1<<bitPos))>>bitPos;
                                            pinMosi.SetAlternatePort( outBit);
                                        }
                                    }
                                    break;

                            } //end of switch 1/4 spi clock
                        } //end of CPHA ==1 
                        else
                        { //CPHA == 0

                            switch (clkCnt%(clkDiv/4)) {

                                case 0:	
                                    if (spcr & DORD) { // LSB first
                                        ror(&spdrRead);
                                        spdrRead&=0x7f;
                                        if (pinMiso) { //pin == 1
                                            spdrRead|=0x80;
                                        }
                                    } else {
                                        rol(&spdrRead);
                                        spdrRead&=0xfe;
                                        if (pinMiso) { //pin == 1
                                            spdrRead|=0x01;
                                        }
                                    }
                                    pinSck.SetAlternatePort(valClkValid);

                                    break;

                                case 1:	
                                    {
                                        bool outBit=(spdrWrite&(1<<bitPos))>>bitPos;
                                        pinMosi.SetAlternatePort( outBit);
                                    }
                                    break;

                                case 2:
                                    pinSck.SetAlternatePort(valClkIdle);	
                                    break;

                                case 3: 
                                    ; //realy nothing? 
                                    break;
                            } //end of switch 1/4 spi clock
                        }

                    }
                    break; //end of case BIT_MASTER

                case BIT_SLAVE:	//only against warning here :-)
                case START_AS_SLAVE:
                    break;
            } //switch state


        } //end of if clkDiv/4 


    } //end of spi is enabled

    return 0;
}

    HWSpi::HWSpi( AvrDevice *_c, HWIrqSystem *is, PinAtPort mo, PinAtPort mi, PinAtPort sc, PinAtPort s, unsigned int vfs): 
Hardware(_c), core(_c), irqSystem(is), pinMosi(mo), pinMiso(mi), pinSck(sc), pinSs(s), vectorForSpif(vfs) 
{
    //core->AddToCycleList(this);
    //irqSystem->RegisterIrqPartner(this, vfs);	//we are assigned for handling irq's with vector no vfs here!
    Reset();
}

void HWSpi::Reset() {
    SetSpcr(0);
    spsr=0;
    spdrWrite=0;
    spdrRead=0;
}

unsigned char HWSpi::GetSpdr() {
    clkCnt=0;
    if (spsr&SPIF) {
        if (spifWeak==1) {
            spsr&=0xff-SPIF-WCOL; 	//if status is read with SPIF == 1
            //we can remove spif flag after reading
            //data register	
        }
    }
    return spdrRead;
}

unsigned char HWSpi::GetSpsr() { 
    spifWeak=1;
    return spsr;
}

unsigned char HWSpi::GetSpcr() { return spcr; }

#if 0
bool HWSpi::IsIrqFlagSet(unsigned int vector) {
    return 1;

    /* this function mjust be removed later
    if (vector== vectorForSpif) {

        return (( spcr & SPIE ) && ( spsr & SPIF) );
    } else {
        cout << "WWWWAAAARRRNNNNIIIINNNNGGG  Warning there is HWSPI called to get a irq vector which is not assigned for!?!?!?!?";
    }
    return 0;
    */

}
#endif

void HWSpi::ClearIrqFlag(unsigned int vector) {
    if (vector== vectorForSpif) {
        spsr&=0xff-SPIF;
        irqSystem->ClearIrqFlag( vectorForSpif);
    } else {
        cout << "WWWWAAAARRRNNNNIIIINNNNGGG  Warning there is HWSPI called to get a irq vector which is not assigned for!?!?!?!?";
    }
}

void HWMegaSpi::SetSpcr(unsigned char val) { 
    spcr=val;

    if ( spcr & SPE) { //SPI is enabled
        if (spcr & MSTR) { //master
            pinMiso.SetUseAlternateDdr(1);
            pinMiso.SetAlternateDdr(0); //ever input
            pinMosi.SetUseAlternatePortIfDdrSet(1);
            pinSck.SetUseAlternatePortIfDdrSet(1);
            if (spcr & CPOL) { // high idle
                pinSck.SetAlternatePort(1);
            } else {
                pinSck.SetAlternatePort(0);
            }

        } else { //slave
            pinMiso.SetUseAlternatePortIfDdrSet(1);
            pinMosi.SetUseAlternateDdr(1);
            pinMosi.SetAlternateDdr(0);
            pinSck.SetUseAlternateDdr(1);
            pinSck.SetAlternateDdr(0);
            pinSs.SetUseAlternateDdr(1);
            pinSs.SetAlternateDdr(0);
        } 
        //selecting the clockDivider

        if ((spsr&SPI2X)==0) {
            switch (spcr & (SPR1|SPR0) ) {
                case 0: clkDiv=4; break;
                case SPR0: clkDiv=16; break;
                case SPR1: clkDiv=64; break;
                case SPR1|SPR0: clkDiv=128; break;
            }
        } else {
            switch (spcr & (SPR1|SPR0) ) {
                case 0: clkDiv=2; break;
                case SPR0: clkDiv=8; break;
                case SPR1: clkDiv=32; break;
                case SPR1|SPR0: clkDiv=64; break;
            }
        }


    } else { //Spi is off so unset alternate pin functions
        pinMosi.SetUseAlternatePortIfDdrSet(0);
        pinMiso.SetUseAlternatePortIfDdrSet(0);
        pinSck.SetUseAlternatePortIfDdrSet(0);
        pinMosi.SetUseAlternateDdr(0);
        pinMiso.SetUseAlternateDdr(0);
        pinSck.SetUseAlternateDdr(0);
        pinSs.SetUseAlternateDdr(0);
    }
}

    HWMegaSpi::HWMegaSpi( AvrDevice *core, HWIrqSystem *is, PinAtPort mo, PinAtPort mi, PinAtPort sc, PinAtPort s, unsigned int vfs): 
HWSpi( core, is, mo, mi, sc, s, vfs) 
{
    core->AddToCycleList(this);
    //irqSystem->RegisterIrqPartner(this, vfs);	//we are assigned for handling irq's with vector no vfs here!
    Reset();
}




void HWMegaSpi::SetSpsr(unsigned char val) { 
    spsr&=(0xff-SPI2X);
    spsr|=val&(0xff-SPI2X);


    if ((spsr&SPI2X)==0) {
        switch (spcr & (SPR1|SPR0) ) {
            case 0: clkDiv=4; break;
            case SPR0: clkDiv=16; break;
            case SPR1: clkDiv=64; break;
            case SPR1|SPR0: clkDiv=128; break;
        }
    } else {
        switch (spcr & (SPR1|SPR0) ) {
            case 0: clkDiv=2; break;
            case SPR0: clkDiv=8; break;
            case SPR1: clkDiv=32; break;
            case SPR1|SPR0: clkDiv=64; break;
        }
    }

} 

unsigned char RWSpdr::operator=(unsigned char val) { trioaccess("Spdr",val);spi->SetSpdr(val);  return val; } 
unsigned char RWSpsr::operator=(unsigned char val) { trioaccess("Spsr",val);spi->SetSpsr(val);  return val; } 
unsigned char RWSpcr::operator=(unsigned char val) { trioaccess("Spcr",val);spi->SetSpcr(val);  return val; } 

RWSpdr::operator unsigned char() const { return spi->GetSpdr(); } 
RWSpsr::operator unsigned char() const { return spi->GetSpsr(); }
RWSpcr::operator unsigned char() const { return spi->GetSpcr(); }
