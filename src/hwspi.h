 /*
 ****************************************************************************
 *
 * simulavr - A simulator for the Atmel AVR family of microcontrollers.
 * Copyright (C) 2001, 2002, 2003   Klaus Rudolph       
 * Copyright (C) 2009 Onno Kortmann
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

#ifndef HWSPI
#define HWSPI

#include "hardware.h"
#include "pinatport.h"
#include "rwmem.h"
#include "traceval.h"

class AvrDevice;
class HWIrqSystem;

/*! Implements the I/O hardware necessary to do SPI transfers. */
class HWSpi: public Hardware, public TraceValueRegister {
    
    private:
        /*! Register into which incoming data is shifted first before
          it ends in spdrRead (double buffer). */
        unsigned char shift_in;
        /*! Contents which appear when SPDR is read. */
        unsigned char data_read;
        /*! Byte to send, accessed by SPDR write. */
        unsigned char data_write;
        unsigned char spsr;
        unsigned char spcr;
    
        AvrDevice *core;
        HWIrqSystem *irq;
    
        PinAtPort MOSI;
        PinAtPort MISO;
        PinAtPort SCK;
        PinAtPort SS;
        unsigned int irq_vector;
    
        /*! Clock divider for SPI transfers; the system clock
          is divided by this amount before being fed to the state logic. */
        int clkdiv;
    
        //! Takes info from registers and updates clkdiv
        void updatePrescaler();
        
        /*! If this is true, SPSR has been read (see for example ATmega 8 DS 10/06,
          p. 131 */
        bool spsr_read;
    
        // For edge detection of SCK in slave mode
        bool oldsck;
    
        /*! Bit counter counting from zero (start bit) to eight (idle). */
        int bitcnt;
    
        /*! Main clock cycles (will be divided to yield SPI clock cycles) */
        unsigned clkcnt;
        
        /* Counter which counts SPI cycles, which is main clk / clkDiv. */
        int spi_cycles;
    
        /*!
          mega mode: Iff true, the SPI2X option becomes available and SPSR will
          be R/W. */
        bool mega_mode;
    
        //! finished transmission?
        bool finished;
        
        //! Send/receive one bit 
        void txbit(const int bitpos);
        void rxbit(const int bitpos);
    
        //! Handle end of transmission if necessary
        void trxend();
    
        //! Called for all SPDR access to clear the WCOL and SPIF flags if needed
        void spdr_access();
        
    public:
        HWSpi(AvrDevice *core,
              HWIrqSystem *,
              PinAtPort mosi,
              PinAtPort miso,
              PinAtPort sck,
              PinAtPort ss,
              unsigned int irq_vec,
              bool mega_mode=true);
        
        unsigned int CpuCycle();
        void Reset();
    
        void SetSPDR(unsigned char val);
        void SetSPSR(unsigned char val); // it is read only! but we need it for rwmem-> only tell that we have an error 
        void SetSPCR(unsigned char val);
    
        unsigned char GetSPDR();
        unsigned char GetSPSR();
        unsigned char GetSPCR();
    
        void ClearIrqFlag(unsigned int);
    
        IOReg<HWSpi> spdr_reg,
                     spsr_reg,
                     spcr_reg;
};

#endif
