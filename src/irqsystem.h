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
#ifndef HWIRQSYSTEM
#define HWIRQSYSTEM

#include <map>
using namespace std;

#include "hwsreg.h"
#include "hardware.h"


class HWIrqSystem {
	protected:
		int bytesPerVector;
		HWSreg *status;

		//setup a stack for hardwareIrqPartners
		map<unsigned int, Hardware *> irqPartnerList;

	public:
		HWIrqSystem (int bytes) { bytesPerVector=bytes; }

		unsigned int GetNewPc(); //if an IRQ occured, we need the new PC,
		//if PC==0xFFFFFFFF there is no IRQ
		unsigned int JumpToVector(unsigned int vector) ;

		void RegisterIrqPartner(Hardware *, unsigned int vector);
		void SetIrqFlag(Hardware *, unsigned int vector);
		void ClearIrqFlag(unsigned int vector);
};

#endif

