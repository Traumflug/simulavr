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

#include "hwuart.h"
#include "helper.h"

//usr & ucsra
#define RXC 0x80
#define TXC 0x40
#define UDRE 0x20
#define FE 0x10
#define OR 0x08
#define DOR 0x08 //same for usart
#define UPE 0x04    //only usart
#define U2X 0x02    //only usart
#define MPCM 0x01   //only usart


//ucr & ucsrb
#define RXCIE 0x80
#define TXCIE 0x40
#define UDRIE 0x20
#define RXEN 0x10
#define TXEN 0x08
#define CHR9 0x04 //same as ucsz2
#define UCSZ2 0x04
#define RXB8 0x02
#define TXB8 0x01

//ussrc usart only
#define URSEL 0x80
#define UMSEL 0x40
#define UPM1  0x20
#define UPM0  0x10
#define USBS  0x08
#define UCSZ1 0x04
#define UCSZ0 0x02
#define UCPOL 0x01

void HWUart::SetUdr(unsigned char val) { 
    udrWrite=val;
    if ( usr&UDRE) { //the data register was empty
        usr &=0xff-UDRE; //so we are not able to send another value now 
        if (ucr & UDRIE) { // UDRE irq was allready set, so clear it
            irqSystem->ClearIrqFlag(vectorUdre);
        }
    }

} 

void HWUart::SetUsr(unsigned char val) { 
    unsigned char usrold=usr;
    usr = val;

    unsigned char irqold= ucr & usrold;
    unsigned char irqnew= ucr & usr;

    if ( usr & TXC) {
        usr &=0xff-TXC; //clear TXC if 1 written to TXC
    }

    unsigned char changed=irqold^irqnew;
    unsigned char setnew= changed&irqnew;
    unsigned char clearnew= changed& (~irqnew);

    CheckForNewSetIrq(setnew);
    CheckForNewClearIrq(clearnew);
} 

void HWUart::SetUbrr(unsigned char val) {
    ubrr = (ubrr & 0xff00) | val;
}

void HWUart::SetUbrrhi(unsigned char val) {
    ubrr = (ubrr & 0xff) | ((val & 0xf) << 8);
}

void HWUart::SetFrameLengthFromRegister() {
    if ( ucr&UCSZ2) {
        frameLength=9;
    } else {
        switch (ucsrc & (UCSZ1|UCSZ0) ) {
            case 0:
                frameLength=5;
                break;

            case UCSZ0:
                frameLength=6;
                break;

            case UCSZ1:
                frameLength=7;
                break;

            case UCSZ0|UCSZ1:
                frameLength=8;
                break;
        }
    }

    frameLength--; // all compares run from 0..frameLength -> -1
}

void HWUart::SetUcr(unsigned char val) { 
    unsigned char ucrold=ucr;
    ucr=val;
    SetFrameLengthFromRegister();

    if (ucr & TXEN) {
        if (txState == TX_FIRST_RUN || txState == TX_SEND_STARTBIT) {
            pinTx.SetAlternatePort(1); //send high bit
        }
        pinTx.SetAlternateDdr(1);       //output!
        pinTx.SetUseAlternatePort(1);
        pinTx.SetUseAlternateDdr(1);
    } else {
        pinTx.SetUseAlternateDdr(0);
        pinTx.SetUseAlternatePort(0);
    }

    if (ucr & RXEN) {
        pinRx.SetUseAlternateDdr(1);
        pinRx.SetAlternateDdr(0);       // input 
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
    baudCnt++; // TODO: this isn't implemented right, baud clock prescaler is a down counter!
    if(baudCnt >= (ubrr + 1)) {
        baudCnt = 0;
        CpuCycleRx();
        CpuCycleTx();
    }

    // controling read sequence down counter
    if(regSeq > 0)
        regSeq--;
      
    return 0;
}

unsigned int HWUart::CpuCycleRx() {
    // receiver part
    //
    //this part MUST! ONLY BE CALLED IF THE PRESCALER OUTPUT COUNTS
    if ( ucr & RXEN) {
        // will be used to cause interrupts (in the end of this if)
        unsigned char usr_old=usr;

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
                        readParity^=1; 
                    }

                    rxBitCnt++;
                    cntRxSamples=0;
                    rxLowCnt=0;
                    rxHighCnt=0;

                    if (rxBitCnt>frameLength) {
                        if (ucsrc&UPM1) {
                            rxState=RX_READ_PARITY;
                        } else {
                            rxState=RX_READ_STOPBIT;
                        }
                    }

                }   

                break;

            case RX_READ_PARITY:
                cntRxSamples++;
                if (cntRxSamples>=8 && cntRxSamples<=10) {
                    if (pinRx==0) {
                        rxLowCnt++;
                    } else {
                        rxHighCnt++;
                    }
                }

                if (cntRxSamples>15) {
                    bool actParity;

                    if ( rxLowCnt<rxHighCnt) { //the bit was high 
                        actParity=1; 
                    } else {
                        actParity=0;
                    }

                    if (ucsrc & UPM0) { //odd parity
                        actParity=!actParity;
                    }

                    if (readParity==actParity) {
                        usr&=0xff-UPE; //clear parity error
                    } else {
                        usr|=UPE;
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

                if (
                        ((ucsrc&USBS) && (cntRxSamples>16)) || //if 2 stopbits used we have to wait full bit frame
                        ((!(ucsrc&USBS)) && (cntRxSamples>10)) //in case of only 1 stopbit we can shorten the stopbit
                   ) {

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
                        udrRead=rxDataTmp&0xff;
                        usr|=FE;
                    }
                    if (ucsrc&USBS) {
                        cntRxSamples=0;
                        rxLowCnt=0;
                        rxHighCnt=0;
                        rxState= RX_READ_STOPBIT2;  
                    } else {
                        if (usr&RXC) { //RXC is allready set->Overrun Error
                            usr|=OR;
                        }
                        usr|=RXC; //receiving is complete, regardless of framing error!
                        if (rxLowCnt<rxHighCnt)
                            rxState = RX_WAIT_FOR_LOWEDGE;
                        else 
                            rxState = RX_WAIT_FOR_HIGH;
                    }
                }   
                break;

            case RX_READ_STOPBIT2:
                cntRxSamples++;
                if (cntRxSamples>=8 && cntRxSamples<=10) {
                    if (pinRx==0) {
                        rxLowCnt++;
                    } else {
                        rxHighCnt++;
                    }
                } else if (cntRxSamples>10) { //the last stopbit could allways be shorter than normal data-bit
                    if ( rxLowCnt<rxHighCnt) { //the bit was high this is ok
                        usr&=0xff-FE;
                    } else { //stopbit was low so set framing error
                        usr|=FE;
                    }
                    
                    usr|=RXC; //receiving is complete, regardless of framing error!
                    if (rxLowCnt<rxHighCnt)
                        rxState = RX_WAIT_FOR_LOWEDGE;
                    else 
                        rxState = RX_WAIT_FOR_HIGH;
                }
                break;

            case RX_DISABLED:
                break;
        } //end of switch

        // check if as a result of this operation any interrupt flags sholud be changed.
        unsigned char irqold= ucr&usr_old;
        unsigned char irqnew= ucr&usr;
     
     
        unsigned char changed=irqold^irqnew;
        unsigned char setnew= changed&irqnew;
        unsigned char clearnew= changed& (~irqnew);
     
        CheckForNewSetIrq(setnew);
        CheckForNewClearIrq(clearnew);
    
    } // end of rx enabled
    return 0;
}

unsigned int HWUart::CpuCycleTx() {
    /*************************************** TRANCEIVER PART **********************************/
    //unsigned char usr_old=usr;

    baudCnt16++;
    if(baudCnt16 >= 16) { // TODO: this isn't implemented right, baud clock prescaler is a down counter!
        baudCnt16 = 0;

        if (ucr & TXEN ) {  //transmitter enabled

            // will be used to cause interrupts int the end of this if
            unsigned char usr_old=usr;

            if (!(usr & UDRE) ) { // there is new data in udr
                if ((usr & TXC)| (txState==TX_FIRST_RUN)|(txState==TX_FINISH)) { //transmitter is empty
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
                    writeParity^= (txDataTmp&(1<<txBitCnt))>>txBitCnt;
                    txBitCnt++;

                    if (txBitCnt>frameLength)  {
                        if (ucsrc & (UPM0|UPM1) ) {
                            txState=TX_SEND_PARITY;
                        } else {
                            txState=TX_SEND_STOPBIT;
                        }
                    }

                    break;

                case TX_SEND_PARITY:
                    if( ucsrc & UPM0) { 
                        //even parity to send
                        pinTx.SetAlternatePort(writeParity);
                    } else {
                        //odd parity to send
                        pinTx.SetAlternatePort(!writeParity);
                    }
                    txState=TX_SEND_STOPBIT;
                    break;


                case TX_SEND_STOPBIT:
                    pinTx.SetAlternatePort(1);

                    if (ucsrc & USBS) { //two stop bits needed?
                        txState=TX_SEND_STOPBIT2;
                    } else {
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
                    }
                    break;

                case TX_SEND_STOPBIT2:
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
                    usr|=TXC;           //set the txc 
                    txState=TX_FINISH; 
                    break;



                case TX_DISABLED:
                case TX_FIRST_RUN:
                case TX_FINISH:
                    break;

            } //end of switch tx state

            // check if some interrupts should be caused as a result
            // of this sending tact
            unsigned char irqold= ucr&usr_old;
            unsigned char irqnew= ucr&usr;


            unsigned char changed=irqold^irqnew;
            unsigned char setnew= changed&irqnew;
            unsigned char clearnew= changed& (~irqnew);

            CheckForNewSetIrq(setnew);
            CheckForNewClearIrq(clearnew);

        } // end of tx enabled 
    }   //end of 1 time baudrate


    return 0;
}

HWUart::HWUart(AvrDevice *core,
               HWIrqSystem *s,
               PinAtPort tx,
               PinAtPort rx,
               unsigned int rx_interrupt,
               unsigned int udre_interrupt,
               unsigned int tx_interrupt,
               int instance_id):
    Hardware(core),
    TraceValueRegister(core, "UART" + int2str(instance_id)),
    irqSystem(s),
    pinTx(tx),
    pinRx(rx),
    vectorRx(rx_interrupt),
    vectorUdre(udre_interrupt),
    vectorTx(tx_interrupt),
    udr_reg(this, "UDR",
            this, &HWUart::GetUdr, &HWUart::SetUdr),
    usr_reg(this, "USR",
            this, &HWUart::GetUsr, &HWUart::SetUsr),
    ucr_reg(this, "UCR",
            this, &HWUart::GetUcr, &HWUart::SetUcr),
    ucsra_reg(this, "UCSRA",
              this, &HWUart::GetUsr, &HWUart::SetUsr),
    ucsrb_reg(this, "UCSRB",
              this, &HWUart::GetUcr, &HWUart::SetUcr),
    ubrr_reg(this, "UBRR",
             this, &HWUart::GetUbrr, &HWUart::SetUbrr),
    ubrrhi_reg(this, "UBRRHI",
               this, &HWUart::GetUbrrhi, &HWUart::SetUbrrhi)
{
    irqSystem->DebugVerifyInterruptVector(vectorRx, this);
    irqSystem->DebugVerifyInterruptVector(vectorUdre, this);
    irqSystem->DebugVerifyInterruptVector(vectorTx, this);

    core->AddToCycleList(this);

    trace_direct(this, "UDR_write", &udrWrite);
    trace_direct(this, "UDR_read", &udrRead);
    trace_direct(this, "sUSR", &usr);
    trace_direct(this, "sUCR", &ucr);
    trace_direct(this, "sUBR", &ubrr);

    Reset();
}

unsigned char HWUart::GetUdr() {
    if (usr&RXC) {
        usr&=0xff-RXC; // unset RXC register
        if (ucr & RXC) {
            irqSystem->ClearIrqFlag(vectorRx); // and clear interrupt flag
        }    
    }
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

void HWUart::Reset() {
    udrWrite = 0;
    udrRead = 0;
    usr = UDRE; //UDRE in USR is set 1 on reset
    ucr = 0;
    ucsrc = UCSZ1 | UCSZ0;
    ubrr = 0;
    baudCnt = 0;
    baudCnt16 = 0;
    
    regSeq = 0;
    
    rxState = RX_WAIT_FOR_LOWEDGE;
    txState = TX_FIRST_RUN;

    SetFrameLengthFromRegister(); 
}

// implementation of HWUsart

void HWUsart::SetUcsrc(unsigned char val) {
    ucsrc=val;
    SetFrameLengthFromRegister();
}

void HWUsart::SetUcsrcUbrrh(unsigned char val) {
    if((val & URSEL) == URSEL) {
        SetUcsrc(val & 0x7f);
    } else {
        SetUbrrhi(val);
    }
}

unsigned char HWUsart::GetUcsrc() { return ucsrc; }

unsigned char HWUsart::GetUcsrcUbrrh() {
    if(regSeq == 0) {
        regSeq = 2;
        return GetUbrrhi();
    } else {
        regSeq = 0;
        return GetUcsrc();
    }
}

HWUsart::HWUsart(AvrDevice *core,
                 HWIrqSystem *s,
                 PinAtPort tx,
                 PinAtPort rx,
                 PinAtPort xck,
                 unsigned int vrx,
                 unsigned int vudre,
                 unsigned int vtx,
                 int instance_id,
                 bool mxReg):
    HWUart(core, s, tx, rx, vrx, vudre, vtx, instance_id),
    pinXck(xck),
    ucsrc_reg(this, "UCSRC",
              this, &HWUsart::GetUcsrc, &HWUsart::SetUcsrc),
    ubrrh_reg(this, "UBRRH",
              this, &HWUsart::GetUbrrhi, &HWUsart::SetUbrrhi),
    ucsrc_ubrrh_reg(this, "UCSRC_UBRRH",
                    this, &HWUsart::GetUcsrcUbrrh, &HWUsart::SetUcsrcUbrrh)
{
    if(mxReg) {
        ucsrc_reg.releaseTraceValue();
        ubrrh_reg.releaseTraceValue();
    } else
        ucsrc_ubrrh_reg.releaseTraceValue();
        
    Reset();
}

// EOF
