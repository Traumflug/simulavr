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
#ifndef HWSPI
#define HWSPI

#include "hardware.h"
#include "pinatport.h"
#include "rwmem.h"

class AvrDevice;
class HWIrqSystem;

class HWSpi: public Hardware {
	protected:
		unsigned char spdrRead;
		unsigned char spdrWrite;
		unsigned char spsr;
		unsigned char spcr;

        AvrDevice *core;
		HWIrqSystem *irqSystem;

		PinAtPort pinMosi;
		PinAtPort pinMiso;
		PinAtPort pinSck;
		PinAtPort pinSs;
		unsigned int vectorForSpif;

		unsigned char clkDiv;

		enum T_State {
			READY,
			START_AS_MASTER,
			BIT_MASTER,
			START_AS_SLAVE,
			BIT_SLAVE
				
		};

		T_State state;

		void rol(unsigned char *);
		void ror(unsigned char *);
		bool spifWeak; 
		bool oldSck; //remember old sck for edge detection of sck in slave mode

        int bitCnt;
        int clkCnt;

	public:
		HWSpi(AvrDevice *core, HWIrqSystem *, PinAtPort mo, PinAtPort mi, PinAtPort sc, PinAtPort s, unsigned int vfs); 
		unsigned int CpuCycle();
		void Reset();

		void SetSpdr(unsigned char val);
		void SetSpsr(unsigned char val); // it is read only! but we need it for rwmem-> only tell that we have an error 
		void SetSpcr(unsigned char val);


		unsigned char GetSpdr();
		unsigned char GetSpsr();
		unsigned char GetSpcr();


		//bool IsIrqFlagSet(unsigned int);
		void ClearIrqFlag(unsigned int);
};

class HWMegaSpi: public HWSpi {
    public:
		HWMegaSpi(AvrDevice *core, HWIrqSystem *, PinAtPort mo, PinAtPort mi, PinAtPort sc, PinAtPort s, unsigned int vfs); 
		void SetSpsr(unsigned char val); // it is NOT read only in mega devices
		void SetSpcr(unsigned char val);

};



class RWSpdr: public RWMemoryMembers {
    protected:
        HWSpi* spi;
    public:
        RWSpdr(HWSpi *s) { spi=s; }
        virtual unsigned char operator=(unsigned char);
        virtual operator unsigned char() const;
};

class RWSpsr: public RWMemoryMembers {
    protected:
        HWSpi* spi;
    public:
        RWSpsr(HWSpi *s) { spi=s; }
        virtual unsigned char operator=(unsigned char);
        virtual operator unsigned char() const;
};

class RWSpcr: public RWMemoryMembers {
    protected:
        HWSpi* spi;
    public:
        RWSpcr(HWSpi *s) { spi=s; }
        virtual unsigned char operator=(unsigned char);
        virtual operator unsigned char() const;
};


#endif
