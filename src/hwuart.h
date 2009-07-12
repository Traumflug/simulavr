/*
 ****************************************************************************
 *
 * simulavr - A simulator for the Atmel AVR family of microcontrollers.
 * Copyright (C) 2001, 2002, 2003, 2004, 2005   Klaus Rudolph		
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

#ifndef HWUART
#define HWUART

#include "trace.h" 
#include "hardware.h"
#include "pinatport.h"
#include "rwmem.h"

class AvrDevice;
class HWIrqSystem;

class HWUart: public Hardware {
    protected:
        unsigned char udrWrite;
        unsigned char udrRead;
        unsigned char usr;  //also used as ucsra
        unsigned char ucr;  //also used as ucsrb
        unsigned char ucsrc; 
        unsigned short ubrr; //16 bit ubrr to fit also 4433 device

        bool readParity;    //the parity for usart
        bool writeParity;

        int frameLength;

        HWIrqSystem *irqSystem;

        PinAtPort pinTx;
        PinAtPort pinRx;

        unsigned int vectorRx;
        unsigned int vectorUdre;
        unsigned int vectorTx;

        int baudCnt;


        enum T_RxState {
            RX_DISABLED,
            RX_WAIT_FOR_HIGH,
            RX_WAIT_FOR_LOWEDGE,
            RX_READ_STARTBIT,
            RX_READ_DATABIT,
            RX_READ_PARITY,
            RX_READ_STOPBIT,
            RX_READ_STOPBIT2
        } ;

        enum T_TxState{
            TX_DISABLED,
            TX_SEND_STARTBIT,
            TX_SEND_DATABIT,
            TX_SEND_PARITY,
            TX_SEND_STOPBIT,
            TX_SEND_STOPBIT2,
            TX_AFTER_STOPBIT,
            TX_FIRST_RUN,
            TX_FINISH
        } ;

        T_RxState rxState;
        T_TxState txState;



        unsigned int CpuCycleRx();
        unsigned int CpuCycleTx();

        int cntRxSamples;
        int rxLowCnt;
        int rxHighCnt;
        unsigned int rxDataTmp;
        int rxBitCnt;

        int baudCnt16;
        unsigned char txDataTmp;
        int txBitCnt;



    public:
        HWUart(AvrDevice *core, HWIrqSystem *, PinAtPort tx, PinAtPort rx, unsigned int vrx, unsigned int vudre, unsigned int vtx, int n=0); // { irqSystem= s;}
        virtual unsigned int CpuCycle();

        void Reset();

        void SetUdr(unsigned char val);  
        void SetUsr(unsigned char val);  
        void SetUcr(unsigned char val);  
        void SetUbrr(unsigned char val);  
        void SetUbrrhi(unsigned char val);  

        unsigned char GetUdr();
        unsigned char GetUsr();
        unsigned char GetUcr();
        unsigned char GetUbrr();
        unsigned char GetUbrrhi();

        //bool IsIrqFlagSet(unsigned int);
        void ClearIrqFlag(unsigned int);
        void CheckForNewSetIrq(unsigned char);
        void CheckForNewClearIrq(unsigned char);

        IOReg<HWUart>
            udr_reg,
            usr_reg,
            ucr_reg,
            ucsra_reg,
            ucsrb_reg,
            ubrr_reg,
            ubrrhi_reg;
    protected:
        void SetFrameLengthFromRegister();
};

class HWUsart: public HWUart {
    protected:
        PinAtPort pinXck;

    public:
        HWUsart(AvrDevice *core, HWIrqSystem *,
		PinAtPort tx, PinAtPort rx, PinAtPort xck,
		unsigned int vrx, unsigned int vudre, unsigned int vtx, int n=0);

        void SetUcsrc(unsigned char val);

        unsigned char GetUcsrc();

        IOReg<HWUsart> ucsrc_reg;
};

#endif
