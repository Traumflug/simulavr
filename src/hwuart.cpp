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
		usr|=0xff-UDRE; //so we are not able to send another value now
	}
} 


void HWUart::SetUsr(unsigned char val) { 
	if ( val & TXC) {
		usr &=0xff-TXC;	//clear TXC if 1 written to TXC
	}
    CheckForIrq();
} 
void HWUart::SetUbrr(unsigned char val) { ubrr=(ubrr&0xff00)|val; } 
void HWUart::SetUbrrhi( unsigned char val) { ubrr=(ubrr&0xff)|(val<<8); }

void HWUart::SetUcr(unsigned char val) { 
	ucr=val;

	if (ucr & TXEN) {
		pinTx.SetUseAlternateDdr(1);
		pinTx.SetAlternateDdr(1); 		//output!
		pinTx.SetUseAlternatePort(1);
	} else {
		pinTx.SetUseAlternateDdr(0);
		pinTx.SetUseAlternatePort(0);
	}

	if (ucr & RXEN) {
		pinRx.SetUseAlternateDdr(1);
		pinRx.SetAlternateDdr(0);		// input 

		rxState=RX_WAIT_FOR_LOWEDGE;

	}
    CheckForIrq();
}

unsigned int HWUart::CpuCycle() {
	// receiver part
	//
	static int cntRxSamples;
	static int rxLowCnt;
	static int rxHighCnt;
	static unsigned int rxDataTmp;
	static int rxBitCnt;
	static int baudCnt;
	static int baudCnt16;
	static unsigned char txDataTmp;
	static int txBitCnt;



	//this part MUST! ONLY BE CALLED IF THE PRESCALER OUTPUT COUNTS
	baudCnt++;
	if (baudCnt== (ubrr+1)) {	//16 times the baud rate!
		baudCnt=0;
		if ( ucr & RXEN) {
			switch (rxState) {
				case RX_WAIT_FOR_HIGH:
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
                        if ( ucr & RXCIE) { irqSystem->SetIrqFlag(this, vectorRx);  }

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
		/*************************************** TRANCEIVER PART **********************************/
		baudCnt16++;
		if (baudCnt16==16) { //1 time baud rate - baud rate / 16 here
			baudCnt16=0;




			if (ucr & TXEN ) {	//transmitter enabled
				if (usr & UDRE ) { // there is new data in udr
					if (usr & TXC) { //transmitter is empty
						//shift data from udr->transmit shift register
						txDataTmp=udrWrite;
						if (ucr & TXB8) { // there is a 1 in txb8
							txDataTmp|=0x100; // this is bit 9 in the datastream
						}

						usr|=UDRE; // set UDRE, UDR is empty now
                        if (ucr & UDRIE) { irqSystem->SetIrqFlag(this, vectorUdre); }
						usr&=0xff-TXC; // the transmitter is not ready
                        irqSystem->ClearIrqFlag( vectorTx);
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
						if (usr & UDRE ) { // there is new data in udr
							//shift data from udr->transmit shift register
							txDataTmp=udrWrite;
							if (ucr & TXB8) { // there is a 1 in txb8
								txDataTmp|=0x100; // this is bit 9 in the datastream
							}

							usr|=UDRE; // set UDRE, UDR is empty now
                            if ( ucr & UDRIE) { irqSystem->SetIrqFlag( this, vectorUdre); }
							usr&=0xff-TXC; // the transmitter is not ready
                            irqSystem->ClearIrqFlag( vectorTx);
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
                        if (ucr&TXCIE) { irqSystem->SetIrqFlag( this, vectorTx);  }
						break;



					case TX_DISABLED:
						break;

				} //end of switch tx state
			} // end of tx enabled 
		}	//end of 1 time baudrate
	} //end of 16 times baud rate



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
	usr=0x20; //UDRE in USR is set 1 on reset
	ucr=0;
	ubrr=0;
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

void HWUart::CheckForIrq() {
    if ((ucr & RXCIE ) && ( usr & RXC))  { irqSystem->SetIrqFlag(this, vectorRx); } else { irqSystem->ClearIrqFlag(vectorRx); }
    if ((ucr & UDRIE ) && ( usr & UDRE)) { irqSystem->SetIrqFlag(this, vectorUdre); } else { irqSystem->ClearIrqFlag(vectorUdre); }
    if ((ucr & TXCIE ) && ( usr & TXC))  { irqSystem->SetIrqFlag(this, vectorTx); } else { irqSystem->ClearIrqFlag(vectorTx); }
}

#if 0
bool HWUart::IsIrqFlagSet(unsigned int vec){
    return 1;

    /* remove that function later
	//cout << "Checking for irq flags in UART";
	if (vec == vectorRx) {
		if ((ucr & RXCIE ) && ( usr & RXC)){
			return 1;
		} else {
			return 0;
		}
	} else if (vec == vectorUdre) {
		if ((ucr & UDRIE ) && ( usr & UDRE)){
			return 1;
		} else {
			return 0;
		}
	} else if (vec == vectorTx) {
		if ((ucr & TXCIE ) && ( usr & TXC)) {
			return 1;
		} else {
			return 0;
		}
	}
	return 0;
    */
}
#endif


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
