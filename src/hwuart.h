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
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 ****************************************************************************
 *
 *  $Id$
 */

#ifndef HWUART
#define HWUART

#include "avrdevice.h"
#include "irqsystem.h"
#include "hardware.h"
#include "pinatport.h"
#include "rwmem.h"
#include "traceval.h"

//! Implements the I/O hardware necessary to do UART transfers.
/*! \todo Needs rewrite! Only one async mode implemented! */
class HWUart: public Hardware, public TraceValueRegister {
    
    protected:
        unsigned char udrWrite; //!< Write stage of UDR register value
        unsigned char udrRead;  //!< Read stage of UDR register value
        unsigned char usr;      //!< USR register value, also used as UCSRA register value
        unsigned char ucr;      //!< UCR register value, also used as UCSRB register value
        unsigned char ucsrc;    //!< UCSRC register value
        unsigned short ubrr;    //!< Baud rate register value (UBRR)

        bool readParity;        //!< The read parity flag for usart
        bool writeParity;       //!< The write parity flag for usart

        int frameLength;        //!< Hold length of UART frame

        HWIrqSystem *irqSystem; //!< Connection to interrupt system

        PinAtPort pinTx;        //!< TX pin
        PinAtPort pinRx;        //!< RX pin

        unsigned int vectorRx;   //!< Interrupt vector ID for receive interrupt
        unsigned int vectorUdre; //!< Interrupt vector ID for UDR empty interrupt
        unsigned int vectorTx;   //!< Interrupt vector ID for sent byte interrupt

        unsigned char regSeq;    //!< Cycle timer for controling read access to UCSRC/UBRRH combined register
        
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
        //! Creates a instance of HWUart class
        HWUart(AvrDevice *core,
               HWIrqSystem *,
               PinAtPort tx,
               PinAtPort rx,
               unsigned int rx_interrupt,
               unsigned int udre_interrupt,
               unsigned int tx_interrupt,
               int instance_id = 0);
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

        void ClearIrqFlag(unsigned int);
        void CheckForNewSetIrq(unsigned char);
        void CheckForNewClearIrq(unsigned char);

        IOReg<HWUart>
            udr_reg,
            usr_reg,
            ucr_reg,
            ucsra_reg,
            ucsrb_reg,
            ubrr_reg,    ///< IO register "UBRRxL" - baudrate
            ubrrhi_reg;  ///< IO register "UBRRxH" - baudrate
            
    protected:
        void SetFrameLengthFromRegister();
};

//! Implements the I/O hardware necessary to do USART transfers.
class HWUsart: public HWUart {
    
    protected:
        PinAtPort pinXck; //!< Clock pin for synchronous mode

    public:
        //! Creates a instance of HWUsart class
        HWUsart(AvrDevice *core,
                HWIrqSystem *,
                PinAtPort tx,
                PinAtPort rx,
                PinAtPort xck,
                unsigned int rx_interrupt,
                unsigned int udre_interrupt,
                unsigned int tx_interrupt,
                int instance_id = 0,
                bool mxReg = true);

        void SetUcsrc(unsigned char val);
        void SetUcsrcUbrrh(unsigned char val);

        unsigned char GetUcsrc();
        unsigned char GetUcsrcUbrrh();

        IOReg<HWUsart> ucsrc_reg,
                       ubrrh_reg,
                       ucsrc_ubrrh_reg;
};

#endif
