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
#include "hwuart.h"
#include "avrdevice.h"
#include "irqsystem.h"



#define UDRE 0x20
#define TXEN 0x08
#define RXEN 0x10
#define RXB8 0x02
#define FE 0x10
#define CHR9 0x04
#define RXC 0x80
#define TXC 0x40
#define TXB8 0x01

#define RXCIE 0x80
#define TXCIE 0x40
#define UDRIE 0x20

void HWUart::SetUdr(unsigned char val) { 
    udrWrite=val;
    if ( usr&UDRE) { //the data register was empty
        usr &=0xff-UDRE; //so we are not able to send another value now 
    }
} 


void HWUart::SetUsr(unsigned char val) { 
    unsigned char usrold=usr;

    if ( val & TXC) {
        usr &=0xff-TXC;	//clear TXC if 1 written to TXC
    }

    unsigned char irqold= ucr&usrold;
    unsigned char irqnew= ucr&usr;


    unsigned char changed=irqold^irqnew;
    unsigned char setnew= changed&irqnew;
    unsigned char clearnew= changed& (~irqnew);

    CheckForNewSetIrq(setnew);
    CheckForNewClearIrq(clearnew);


} 

void HWUart::SetUbrr(unsigned char val) { ubrr=(ubrr&0xff00)|val; } 
void HWUart::SetUbrrhi( unsigned char val) { ubrr=(ubrr&0xff)|(val<<8); }

void HWUart::SetUcr(unsigned char val) { 
    unsigned char ucrold=ucr;
    ucr=val;

    if (ucr & TXEN) {
        if (txState == TX_FIRST_RUN || txState == TX_SEND_STARTBIT) {
            pinTx.SetAlternatePort(1); //send high bit
        }
        pinTx.SetAlternateDdr(1); 		//output!
        pinTx.SetUseAlternatePort(1);
        pinTx.SetUseAlternateDdr(1);
    } else {
        pinTx.SetUseAlternateDdr(0);
        pinTx.SetUseAlternatePort(0);
    }

    if (ucr & RXEN) {
        pinRx.SetUseAlternateDdr(1);
        pinRx.SetAlternateDdr(0);		// input 
    }
    //prepared for later remove from hwuart from every cpu cycle (only on demand)
#if 0
    //Check if one of Rx or Tx is enabled NEW!
    if ( ucr & ( RXEN|TXEN) ) {//now one of rx or tx is on
        if ( !(ucrold & (RXEN|TXEN)) ) { //but there was nothing on before
            core->AddToCycleList(this); //enable the cpu cycle list!
        }
    }
#endif

    unsigned char irqold= ucrold&usr;
    unsigned char irqnew= ucr&usr;


    unsigned char changed=irqold^irqnew;
    unsigned char setnew= changed&irqnew;
    unsigned char clearnew= changed& (~irqnew);

    CheckForNewSetIrq(setnew);
    CheckForNewClearIrq(clearnew);
}

unsigned int HWUart::CpuCycle() {
    baudCnt++;
    if (baudCnt== (ubrr+1)) {	//16 times the baud rate!
        baudCnt=0;
        CpuCycleRx();
        CpuCycleTx();
    }

    return 0;
}
unsigned int HWUart::CpuCycleRx() {
    // receiver part
    //
    //this part MUST! ONLY BE CALLED IF THE PRESCALER OUTPUT COUNTS
    if ( ucr & RXEN) {
        switch (rxState) {
            case RX_WAIT_FOR_HIGH: //wait for startbit
                if (pinRx==1) rxState=RX_WAIT_FOR_LOWEDGE;
                break;

            case RX_WAIT_FOR_LOWEDGE:
                if (pinRx==0) rxState=RX_READ_STARTBIT;
                cntRxSamples=0;
                rxLowCnt=0;
                rxHighCnt=0;
                break;

            case RX_READ_STARTBIT:
                cntRxSamples++;
                if (cntRxSamples>=8 && cntRxSamples<=10) {
                    if (pinRx==0) {
                        rxLowCnt++;
                    } else {
                        rxHighCnt++;
                    }
                }
                if (cntRxSamples>15) {
                    if ( rxLowCnt>rxHighCnt) { //yes the startbit is low 
                        cntRxSamples=0;
                        rxState= RX_READ_DATABIT;
                        rxDataTmp=0;
                        rxLowCnt=0;
                        rxHighCnt=0;
                        rxBitCnt=0;
                    } else {
                        rxState=RX_WAIT_FOR_HIGH;
                    }
                }	
                break;

            case RX_READ_DATABIT:
                cntRxSamples++;
                if (cntRxSamples>=8 && cntRxSamples<=10) {
                    if (pinRx==0) {
                        rxLowCnt++;
                    } else {
                        rxHighCnt++;
                    }
                }

                if (cntRxSamples>15) {
                    if ( rxLowCnt<rxHighCnt) { //the bit was high
                        rxDataTmp|=(1<<rxBitCnt);
                    }

                    rxBitCnt++;
                    cntRxSamples=0;
                    rxLowCnt=0;
                    rxHighCnt=0;

                    if ((rxBitCnt>7) && ( (ucr & CHR9) ==0)) {	//8 bits received
                        rxState=RX_READ_STOPBIT;
                    }

                    if (rxBitCnt>8) {		// 9 bits received
                        rxState=RX_READ_STOPBIT;
                    }


                }	

                break;

            case RX_READ_STOPBIT:
                cntRxSamples++;
                if (cntRxSamples>=8 && cntRxSamples<=10) {
                    if (pinRx==0) {
                        rxLowCnt++;
                    } else {
                        rxHighCnt++;
                    }
                }

                if (cntRxSamples>15) {
                    usr|=RXC; //receiving is complete, regardless of framing error!

                    if ( rxLowCnt<rxHighCnt) { //the bit was high this is ok
                        udrRead=rxDataTmp&0xff;
                        usr&=0xff-FE;
                        if ( (ucr & CHR9) !=0 ) {
                            if ( rxDataTmp&0x100) { 
                                ucr|=RXB8;
                            } else {
                                ucr&=0xff-RXB8;
                            }

                        }
                    } else { //stopbit was low so set framing error
                        usr|=FE;
                    }
                    rxState= RX_WAIT_FOR_HIGH;

                }	

                break;

            case RX_DISABLED:
                break;
        } //end of switch
    } // end of rx enabled
    return 0;
}


unsigned int HWUart::CpuCycleTx() {
    /*************************************** TRANCEIVER PART **********************************/
    unsigned char usr_old=usr;

    baudCnt16++;
    if (baudCnt16==16) { //1 time baud rate - baud rate / 16 here
        baudCnt16=0;

        if (ucr & TXEN ) {	//transmitter enabled
            if (!(usr & UDRE) ) { // there is new data in udr
                if ((usr & TXC)| (txState==TX_FIRST_RUN)) { //transmitter is empty
                    //shift data from udr->transmit shift register
                    txDataTmp=udrWrite;
                    if (ucr & TXB8) { // there is a 1 in txb8
                        txDataTmp|=0x100; // this is bit 9 in the datastream
                    }


                    usr|=UDRE; // set UDRE, UDR is empty now
                    usr&=0xff-TXC; // the transmitter is not ready
                    txState=TX_SEND_STARTBIT;
                } // end of transmitter empty
            } // end of new data in udr





            switch (txState) {
                case TX_SEND_STARTBIT:
                    pinTx.SetAlternatePort(0);
                    txState=TX_SEND_DATABIT;
                    txBitCnt=0;
                    break;

                case TX_SEND_DATABIT:
                    pinTx.SetAlternatePort((txDataTmp&(1<<txBitCnt))>>txBitCnt);
                    txBitCnt++;

                    if ((txBitCnt>7) && ((ucr & CHR9)==0)) {
                        txState=TX_SEND_STOPBIT;
                    }

                    if (txBitCnt>8) {
                        txState=TX_SEND_STOPBIT;
                    }

                    break;

                case TX_SEND_STOPBIT:
                    pinTx.SetAlternatePort(1);
                    //check for new data
                    if (!(usr & UDRE)) { // there is new data in udr
                        //shift data from udr->transmit shift register
                        txDataTmp=udrWrite;
                        if (ucr & TXB8) { // there is a 1 in txb8
                            txDataTmp|=0x100; // this is bit 9 in the datastream
                        }

                        usr|=UDRE; // set UDRE, UDR is empty now
                        txState=TX_SEND_STARTBIT;
                    } // end of new data in udr
                    else 
                    { 
                        txState=TX_AFTER_STOPBIT;
                    }
                    break;

                case TX_AFTER_STOPBIT: //transmit complete and no new data
                    //leave state untouched
                    usr|=TXC; 			//set the txc 
                    break;



                case TX_DISABLED:
                case TX_FIRST_RUN:
                    break;

            } //end of switch tx state
        } // end of tx enabled 
    }	//end of 1 time baudrate

    unsigned char irqold= ucr&usr_old;
    unsigned char irqnew= ucr&usr;


    unsigned char changed=irqold^irqnew;
    unsigned char setnew= changed&irqnew;
    unsigned char clearnew= changed& (~irqnew);

    CheckForNewSetIrq(setnew);
    CheckForNewClearIrq(clearnew);


    return 0;
}

HWUart::HWUart( AvrDevice *core, HWIrqSystem *s, PinAtPort tx, PinAtPort rx, unsigned int vrx, unsigned int vudre, unsigned int vtx):
Hardware(core), irqSystem(s), pinTx(tx), pinRx(rx), vectorRx(vrx), vectorUdre(vudre), vectorTx(vtx) {
    core->AddToCycleList(this);
    //irqSystem->RegisterIrqPartner(this, vectorRx);
    //irqSystem->RegisterIrqPartner(this, vectorUdre);
    //irqSystem->RegisterIrqPartner(this, vectorTx);

    Reset();
}

void HWUart::Reset() {
    udrWrite=0;
    udrRead=0;
    usr=UDRE; //UDRE in USR is set 1 on reset
    ucr=0;
    ubrr=0;
    rxState=RX_WAIT_FOR_LOWEDGE;
    txState=TX_FIRST_RUN;
}

unsigned char HWUart::GetUdr() { 
    usr&=0xff-RXC; // unset RXC register
    return udrRead;
}
unsigned char HWUart::GetUsr() { return usr; }
unsigned char HWUart::GetUcr() { return ucr; }
unsigned char HWUart::GetUbrr() { return ubrr&0xff; }
unsigned char HWUart::GetUbrrhi() { return (ubrr&0xff00)>>8; }

void HWUart::ClearIrqFlag(unsigned int vector){
    //other Uart IRQ Flags can't be cleared by executing the vector here
    if (vector == vectorTx) {
        usr&=0xff-TXC;
        irqSystem->ClearIrqFlag( vectorTx);
    }
}

void HWUart::CheckForNewSetIrq(unsigned char val) {
    if (val & RXC) { irqSystem->SetIrqFlag(this, vectorRx); }
    if (val & UDRE) { irqSystem->SetIrqFlag(this, vectorUdre); }
    if (val & TXC) { irqSystem->SetIrqFlag(this, vectorTx); }
}

void HWUart::CheckForNewClearIrq(unsigned char val) {
    if (val & RXC) { irqSystem->ClearIrqFlag(vectorRx); }
    if (val & UDRE) { irqSystem->ClearIrqFlag(vectorUdre); }
    if (val & TXC) { irqSystem->ClearIrqFlag(vectorTx); }
}


unsigned char RWUdr::operator=(unsigned char val) { trioaccess("Udr",val);uart->SetUdr(val);  return val; } 
unsigned char RWUsr::operator=(unsigned char val) { trioaccess("Usr",val);uart->SetUsr(val);  return val; } 
unsigned char RWUcr::operator=(unsigned char val) { trioaccess("Ucr",val);uart->SetUcr(val);  return val; } 
unsigned char RWUbrr::operator=(unsigned char val) { trioaccess("Ubrr",val);uart->SetUbrr(val); return val; } 
unsigned char RWUbrrhi::operator=(unsigned char val) { trioaccess("Ubrrhi",val);uart->SetUbrrhi(val); return val; } 


RWUdr::operator unsigned char() const { return uart->GetUdr(); } 
RWUsr::operator unsigned char() const { return uart->GetUsr(); } 
RWUcr::operator unsigned char() const { return uart->GetUcr(); } 
RWUbrr::operator unsigned char() const { return uart->GetUbrr(); } 
RWUbrrhi::operator unsigned char() const { return uart->GetUbrrhi(); } 
