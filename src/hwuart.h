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
        unsigned char usr;
        unsigned char ucr;
        unsigned short ubrr; //16 bit ubrr to fit also 4433 device

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
            RX_READ_STOPBIT
        } ;

        enum T_TxState{
            TX_DISABLED,
            TX_SEND_STARTBIT,
            TX_SEND_DATABIT,
            TX_SEND_STOPBIT,
            TX_AFTER_STOPBIT,
            TX_FIRST_RUN
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
        HWUart(AvrDevice *core, HWIrqSystem *, PinAtPort tx, PinAtPort rx, unsigned int vrx, unsigned int vudre, unsigned int vtx); // { irqSystem= s;}
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
};




class RWUdr: public RWMemoryMembers {
    protected:
        HWUart* uart;
    public:
        RWUdr( HWUart* u) { uart=u; }
        virtual unsigned char operator=(unsigned char);
        virtual operator unsigned char() const;
};
class RWUsr: public RWMemoryMembers {
    protected:
        HWUart* uart;
    public:
        RWUsr( HWUart* u) { uart=u; }
        virtual unsigned char operator=(unsigned char);
        virtual operator unsigned char() const;
};
class RWUcr: public RWMemoryMembers {
    protected:
        HWUart* uart;
    public:
        RWUcr( HWUart* u) { uart=u; }
        virtual unsigned char operator=(unsigned char);
        virtual operator unsigned char() const;
};
class RWUbrr: public RWMemoryMembers {
    protected:
        HWUart* uart;
    public:
        RWUbrr( HWUart* u) { uart=u; }
        virtual unsigned char operator=(unsigned char);
        virtual operator unsigned char() const;
};

class RWUbrrhi: public RWMemoryMembers {
    protected:
        HWUart* uart;
    public:
        RWUbrrhi( HWUart* u) { uart=u; }
        virtual unsigned char operator=(unsigned char);
        virtual operator unsigned char() const;
};

#endif
